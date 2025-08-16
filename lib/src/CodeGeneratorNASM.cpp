//
// Created by doray on 8/14/25.
//

#include "CodeGeneratorNASM.h"

#include <variant>

#include "instructionSelectionTable.h"
#include "TACGenerator.h"
#include "tool.h"
#include "fmt/ranges.h"

namespace ork::codeGenerator {
    void NASM::generate(const std::vector<Instruction> &insts) {
        liveIntervalsAnalysis(insts);

        std::vector<std::string> str;
        for (const auto &inst: insts)
            switch (inst.op) {
                case Operation::ALLOCA: {
                    auto str_result = getVar(inst.result);

                    if (!str_result) {
                        throw std::runtime_error("NASM::generate: result variable not found in liveInterval");
                    }

                    auto arg = resolveOperand(inst.arg1);
                    Register *result_reg = resolveByArg(*str_result, arg);

                    std::string arg_result;
                    if (std::holds_alternative<Register *>(arg)) {
                        arg_result = std::get<Register *>(arg)->name;
                    } else if (std::holds_alternative<std::string>(arg)) {
                        arg_result = std::get<std::string>(arg);
                    }
                    str.emplace_back(fmt::format(
                            fmt::runtime(instructionSelectionTable::NASM_REG[Operation::ALLOCA]),
                            result_reg->name,
                            arg_result)
                    );
                }
                default:
                    break;
            }
        for (auto s: str) {
            fmt::println("{}", s);
        }
    }

    void NASM::liveIntervalsAnalysis(const std::vector<Instruction> &insts) {
        auto updateInterval = [&](const std::optional<std::string> &var, size_t index) {
            if (!var) return;
            auto it = liveInterval.find(*var);
            if (it != liveInterval.end()) {
                it->second.second = index;
            } else {
                liveInterval[*var] = {index, index};
            }
        };

        for (size_t i = 0; i < insts.size(); ++i) {
            updateInterval(getVar(insts[i].result), i);
            updateInterval(getVar(insts[i].arg1), i);
            updateInterval(getVar(insts[i].arg2), i);
        }
    }

    std::optional<Register *> NASM::allocateRegister() {
        for (auto &reg: registers) {
            if (!reg.var_id) {
                return &reg;
            }
        }
        return std::nullopt;
    }

    std::optional<Register *> NASM::findRegisterByVarId(int var_id) {
        for (auto &arg: registers) {
            if (arg.var_id == var_id) {
                return &arg;
            }
        }
        return std::nullopt;
    }

    std::variant<Register *, std::string> NASM::resolveOperand(const std::unique_ptr<VarName>& arg) {
        if (auto str_arg = getVar(arg)) {
            auto f_arg = liveInterval.find(*str_arg);

            if (f_arg == liveInterval.end()) {
                throw std::runtime_error("NASM::generate: arg variable not found in liveInterval");
            }

            if (auto r = findRegisterByVarId(f_arg->second.first)) {
                return *r;
            }

            throw std::runtime_error("NASM::resolveOperand: register for variable not allocated");
        }
        if (auto v = getValue(arg)) {
            return *v;
        }
        throw std::runtime_error("NASM::generate: literal value operand not found");
    }

    Register *NASM::resolveByArg(const std::string &result_var, std::variant<Register *, std::string> arg) {
        auto it = liveInterval.find(result_var);

        if (it == liveInterval.end()) {
            throw std::runtime_error("NASM::generate: arg variable not found in liveInterval");
        }

        auto [start, end] = it->second;

        std::optional<Register *> result_reg = std::visit(
            match{
                [start, end](Register *reg) -> std::optional<Register *> {
                    if (reg && reg->liveIntervalEnd == start) {
                        reg->var_id = start;
                        reg->liveIntervalEnd = end;
                        return reg;
                    }

                    return std::nullopt;
                },
                [](auto &&) -> std::optional<Register *> {
                    // std::string ...
                    return std::nullopt;
                },
            }, arg);

        if (!result_reg) {
            auto optReg = allocateRegister();
            if (!optReg) {
                throw std::runtime_error("NASM::resolveRegisterByVar: no free register available");
            }

            Register *final_reg = *optReg;
            final_reg->var_id = start;
            final_reg->liveIntervalEnd = end;
            return final_reg;
        }
        return *result_reg;
    }

    std::optional<std::string> NASM::getVar(const std::unique_ptr<VarName> &a) {
        if (!a) return std::nullopt;
        return std::visit(
            match{
                [](expression::Identifier id) { return std::optional(id.name); },
                [](std::string s) { return std::optional(s); },
                [](auto &&) -> std::optional<std::string> { return std::nullopt; }
            }, *a);
    }

    std::optional<std::string> NASM::getValue(const std::unique_ptr<VarName> &a) {
        if (!a) return std::nullopt;

        return std::visit(
            match{
                [](Value v) -> std::optional<std::string> {
                    if (std::holds_alternative<int>(v)) {
                        return std::to_string(std::get<int>(v));
                    }
                },
                [](auto &&) -> std::optional<std::string> {
                    return std::nullopt;
                }
            }, *a);
    }
} // namespace ork::codeGenerator

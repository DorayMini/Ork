//
// Created by doray on 8/14/25.
//

#include "CodeGeneratorNASM.h"

#include "instructionSelectionTable.h"
#include "TACGenerator.h"
#include "tool.h"
#include "fmt/ranges.h"

namespace ork::codeGenerator {
    void NASM::generate(const std::vector<Instruction> &insts) {
        liveIntervalsAnalysis(insts);

        std::vector<std::string> str;
        for (size_t i = 0; i < insts.size(); ++i) {
            auto &inst = insts[i];
            switch (inst.op) {
                case Operation::ALLOCA: {
                    auto str_result = getVar(inst.result);

                    if (!str_result) {
                        throw std::runtime_error("NASM::generate: result variable not found in liveInterval");
                    }

                    Register *reg = nullptr;

                    if (auto str_arg = getVar(inst.arg1)) {
                        auto f_result = liveInterval.find(*str_result);
                        auto f_arg = liveInterval.find(*str_arg);

                        if (f_arg == liveInterval.end() || f_result == liveInterval.end()) {
                            throw std::runtime_error("NASM::generate: arg variable not found in liveInterval");
                        }

                        if (f_result->second.first == f_arg->second.second) {
                            if (auto r = findRegisterByVarId(f_arg->second.first)) {
                                (*r)->var_id = f_result->second.first;
                                break;
                            }
                        }

                        auto regOpt = allocateRegister();
                        if (!regOpt)
                            throw std::runtime_error(
                                "NASM::generate: source register for argument variable not found");
                        reg = regOpt.value();
                        if (auto r = findRegisterByVarId(f_arg->second.first)) {
                            str.push_back(fmt::format(fmt::runtime(instructionSelectionTable::NASM_REG[inst.op]),
                                                      reg->name, (*r)->name));
                        }
                    } else {
                        auto regOpt = allocateRegister();
                        if (!regOpt) throw std::runtime_error("NASM::generate: no free register available for literal");
                        reg = regOpt.value();

                        if (auto v = getValue(inst.arg1)) {
                            str.push_back(fmt::format(fmt::runtime(instructionSelectionTable::NASM_REG[inst.op]),
                                                      reg->name, *v));
                        } else {
                            throw std::runtime_error("NASM::generate: literal value operand not found");
                        }
                    }
                    break;
                }

                default:
                    break;
            }
        }
        for (auto s: str) {
            fmt::println("{}", s);
        }
    }

    void NASM::liveIntervalsAnalysis(const std::vector<Instruction>& insts) {
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

    std::optional<std::string> NASM::getVar(auto &&a) {
        if (!a) return std::nullopt;
        return std::visit(
            match{
                [](expression::Identifier id) { return std::optional(id.name); },
                [](std::string s) { return std::optional(s); },
                [](auto &&) -> std::optional<std::string> { return std::nullopt; }
            }, *a);
    }

    std::optional<std::string> NASM::getValue(auto &&a) {
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

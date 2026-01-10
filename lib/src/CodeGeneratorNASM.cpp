//
// Created by doray on 8/14/25.
//

#include "CodeGeneratorNASM.h"

#include <ranges>
#include <variant>

#include "TACGenerator.h"
#include "instructionSelectionTable.h"
#include "tool.h"

namespace ork::codeGenerator {

    /* TODO:
     *  Detect use before definition of variables (Semantic analyzer)
     */
    std::vector<std::string> NASM::generate(const std::vector<Instruction> &insts) {
        liveIntervalsAnalysis(insts);

        size_t index = 0;
        size_t startIndex = 0;
        for (const auto &inst: insts) {
            switch (inst.op) {
                case Operation::ALLOCA:
                    if (findLocation(getOperand(inst.result).value()).has_value()) break;

                    if (hasFreeRegister()) {
                        allocateReg(getOperand(inst.result).value(), index);
                    } else {
                        allocateStack(getOperand(inst.result).value(), 4, index);
                    }
                    break;
                case Operation::FUNC_START:
                    startIndex = index;
                    break;
                case Operation::FUNC_END:
                    generateFuncNasm(insts.begin() + startIndex, insts.begin() + index);
                    varLocation.clear();
                    freeAllReg();
                    stack.offset = 0;
                    break;
                default:
                    throw std::runtime_error("NASM::generate: unsupported operation");
            }
            index++;
        }
        return nasmCode;
    }

    void NASM::generateFuncNasm(std::vector<Instruction>::const_iterator begin, std::vector<Instruction>::const_iterator end) {
        for (auto inst = begin; inst <= end; ++inst) {
            switch (inst->op) {
                case Operation::FUNC_START: {
                    if (varLocation.empty()) break;
                    nasmCode.push_back(
                        fmt::format(
                            fmt::runtime(instructionSelectionTable::NASM_REG[inst->op]),
                            getOperand(inst->result).value()
                        )
                    );

                    std::vector<std::string> stackInitialization;
                    if (stack.offset != 0) stackInitialization = formatStackInitialization();
                    nasmCode.insert(nasmCode.end(), stackInitialization.begin(), stackInitialization.end());
                    break;
                }
                case Operation::FUNC_END: {
                    if (varLocation.empty()) break;
                    std::vector<std::string> stackFinalization;
                    if (stack.offset != 0) stackFinalization = formatStackFinalization();

                    nasmCode.insert(nasmCode.end(), stackFinalization.begin(), stackFinalization.end());
                    nasmCode.push_back(
                        instructionSelectionTable::NASM_REG[inst->op]
                    );
                    break;
                }

                case Operation::ALLOCA: {
                    if (!findLocation(getOperand(inst->result).value()).has_value()) break;

                    std::string resultLoc = formatLocation(findLocation(getOperand(inst->result).value()).value());
                    if (getNumOperand(inst->arg1).has_value()) {
                        nasmCode.push_back(
                        fmt::format(
                            fmt::runtime(instructionSelectionTable::NASM_REG[inst->op]),
                                resultLoc,
                                getNumOperand(inst->arg1).value()
                            )
                        );
                    }
                    else if (getOperand(inst->arg1).has_value()) {
                        auto location = findLocation(getOperand(inst->arg1).value());
                        if (!location.has_value()) throw std::runtime_error("NASM::generateFuncNasm: operand not found");

                        if (location.value().kind == Location::STACK) {
                            auto loc = formatLocation(location.value());
                            nasmCode.push_back(
                                fmt::format(
                                    fmt::runtime(instructionSelectionTable::NASM_REG[inst->op]),
                                    TEMP_REG,
                                    loc
                                )
                            );
                        }

                        nasmCode.push_back(
                        fmt::format(
                            fmt::runtime(instructionSelectionTable::NASM_REG[inst->op]),
                            resultLoc,
                            TEMP_REG
                            )
                        );
                    } else
                        throw std::runtime_error("NASM::generateFuncNasm: unsupported operand type");
                    break;
                }
                default:
                    throw std::runtime_error("NASM::generateFuncNasm: unsupported operation");
            }
        }
    }


    void NASM::liveIntervalsAnalysis(const std::vector<Instruction> &insts) {
        auto updateInterval = [&](const std::optional<std::string> &var, size_t index) {
            if (!var)
                return;
            auto it = liveInterval.find(*var);
            if (it != liveInterval.end()) {
                it->second.second = index;
            } else {
                liveInterval[*var] = {index, index};
            }
        };

        for (size_t i = 0; i < insts.size(); ++i) {
            switch (insts[i].op) {
                case Operation::FUNC_START:
                case Operation::FUNC_END:
                    continue;
                default:
                    break;
            }
            updateInterval(getOperand(insts[i].result), i);
            updateInterval(getOperand(insts[i].arg1), i);
            updateInterval(getOperand(insts[i].arg2), i);
        }
    }

    std::optional<std::string> NASM::getOperand(const std::unique_ptr<Operand> &a) {
        if (!a)
            return std::nullopt;
        return std::visit(
            match{
                [](expression::Identifier id) { return std::optional(id.name); },
                [](std::string s) { return std::optional(s); },
                [](auto &&) -> std::optional<std::string> { return std::nullopt; }
            },
            *a);
    }

    std::optional<int32_t> NASM::getNumOperand(const std::unique_ptr<TACGenerator::Operand> &a) {
        if (!a)
            return std::nullopt;
        return std::visit(
            match{
                [](Value i) { return std::optional(std::get<int32_t>(i)); },
                [](auto &&) -> std::optional<int32_t> { return std::nullopt; }
            },
            *a);
    }

    std::optional<std::string> NASM::allocateReg(const std::string &var, size_t index) {
        freeReg(index);
        if (auto [first, second] = liveInterval[var]; first == second) return std::nullopt;

        for (auto &reg: variableRegs) {
            if (!reg.second) {
                reg.second = true;
                varLocation[var] = Location::regLoc(reg.first);
                return reg.first;
            }
        }

        return std::nullopt;
    }

    std::optional<int> NASM::allocateStack(const std::string &var, int align, size_t index) {
        freeReg(index);
        if (auto [first, second] = liveInterval[var]; first == second) return std::nullopt;

        varLocation[var] = Location::stackLoc(stack.offset);
        stack.offset += align;

        return stack.offset;
    }

    std::optional<Location> NASM::findLocation(const std::string &var) {
        if (auto varLoc = varLocation.find(var); varLoc != varLocation.end()) {
            return varLoc->second;
        }
        return std::nullopt;
    }

    std::string NASM::formatLocation(const Location &loc) {
        switch (loc.kind) {
            case Location::REG:
                return loc.reg;
            case Location::STACK:
                return fmt::format("dword [esp + {}]", loc.offset);
            default:
                throw std::runtime_error("NASM::formatLocation: invalid location kind");
        }
    }

    std::vector<std::string> NASM::formatStackInitialization() {
        std::vector<std::string> result;
        result.emplace_back("push ebp\n");
        result.emplace_back("mov ebp, esp\n");
        int stack_size = (stack.offset / 16 + 1) * 16;
        result.push_back(fmt::format("sub esp, {}\n", stack_size));

        return result;
    }

    std::vector<std::string> NASM::formatStackFinalization() {
        std::vector<std::string> result;
        result.emplace_back("mov esp, ebp\n");
        result.emplace_back("pop ebp\n");

        return result;
    }

    void NASM::freeReg(size_t index) {
        for (auto &loc: varLocation) {
            if (loc.second.kind == Location::STACK) continue;

            if (index > liveInterval[loc.first].second) {
                variableRegs[loc.second.reg] = false;
            }
        }
    }

    bool NASM::hasFreeRegister() {
        for (const auto &val: variableRegs | std::views::values) {
            if (!val) return true;
        }
        return false;
    }

    void NASM::freeAllReg() {
        for (auto& reg: variableRegs | std::views::values) {
            reg = false;
        }
    }
} // namespace ork::codeGenerator

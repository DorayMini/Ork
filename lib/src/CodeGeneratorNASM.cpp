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
                case Operation::FUNC_START:
                    nasmCode.push_back(
                        fmt::format(
                            fmt::runtime(instructionSelectionTable::NASM_REG[inst->op]),
                            getOperand(inst->result).value()
                        )
                    );

                    break;
                case Operation::FUNC_END:
                    nasmCode.push_back(
                        instructionSelectionTable::NASM_REG[inst->op]
                    );
                    break;
                case Operation::ALLOCA:
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
                        auto loc = formatLocation(findLocation(getOperand(inst->arg1).value()).value());
                        nasmCode.push_back(
                        fmt::format(
                            fmt::runtime(instructionSelectionTable::NASM_REG[inst->op]),
                            resultLoc,
                            loc
                            )
                        );
                    } else
                        throw std::runtime_error("NASM::generateFuncNasm: unsupported operand type");
                    break;
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

        for (auto &reg: variableRegs) {
            if (!reg.second) {
                reg.second = true;
                varLocation[var] = Location::regLoc(reg.first);
                return reg.first;
            }
        }

        return std::nullopt;
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

    std::string NASM::formatStackInitialization() {

    }

    int NASM::allocateStack(const std::string &var, int align, size_t index) {
        freeReg(index);

        varLocation[var] = Location::stackLoc(stack.offset);
        stack.offset += align;

        return stack.offset;
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

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
     *  - add SUB inst
     *  Detect use before definition of variables (Semantic analyzer)
     */
    std::vector<std::string> NASM::generate(const std::vector<Instruction> &insts) {
        liveIntervalsAnalysis(insts);

        size_t index = 0;
        size_t startIndex = 0;
        for (const auto &inst: insts) {
            switch (inst.op) {
                case Operation::ADD:
                case Operation::MOV: {
                    auto resultOpOpt = getOperand(inst.result);
                    if (!resultOpOpt) break;
                    if (findLocation(*resultOpOpt)) break;

                    if (hasFreeRegister()) {
                        allocateReg(*resultOpOpt, index);
                    } else {
                        allocateStack(*resultOpOpt, 4, index);
                    }
                    break;
                }
                case Operation::FUNC_START:
                    startIndex = index;
                    break;
                case Operation::FUNC_END:
                    generateFuncNasm(insts.begin() + startIndex, insts.begin() + index);
                    varLocation.clear();
                    freeAllReg();
                    stack.offset = 0;
                    break;
            }
            index++;
        }
        return nasmCode;
    }

    void NASM::generateFuncNasm(std::vector<Instruction>::const_iterator begin, std::vector<Instruction>::const_iterator end) {
        auto emit = [&](const Operation &op, auto&&... args) {
            nasmCode.push_back(
                fmt::format(fmt::runtime(instructionSelectionTable::NASM_REG[op]), std::forward<decltype(args)>(args)...)
             );
        };

        auto emitAloca = [&](const Operation &op, const std::string &dst, const std::string&src, bool useTemp = false) {
            if (useTemp) {
                emit(Operation::MOV, TEMP_REG, src);
                emit(op, dst, TEMP_REG);
            } else {
                emit(op, dst, src);
            }
        };



        for (auto inst = begin; inst <= end; ++inst) {
            switch (inst->op) {
                case Operation::FUNC_START: {
                    if (varLocation.empty()) break;
                    emit(inst->op, getOperand(inst->result).value());

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
                case Operation::MOV: {
                    auto resultOpOpt = getOperand(inst->result);
                    if (!resultOpOpt) break;

                    auto resultLocOpt = findLocation(*resultOpOpt);
                    if (!resultLocOpt) break;

                    std::string resultFormattedLoc = formatLocation(*resultLocOpt);

                    if (auto numOP = getNumOperand(inst->arg1)) {
                        emit(inst->op,resultFormattedLoc, *numOP);
                        break;
                    }

                    auto argOpOpt = getOperand(inst->arg1);
                    if (!argOpOpt) throw std::runtime_error("NASM::generate: unsupported operand type");

                    auto location = findLocation(*argOpOpt);
                    if (!location) throw std::runtime_error("NASM::generateFuncNasm: operand not found");

                    auto formattedLoc = formatLocation(*location);
                    emitAloca(Operation::MOV, resultFormattedLoc, formattedLoc, location->kind == Location::STACK);
                    break;
                }
                case Operation::ADD: {
                    auto resultOpOpt = getOperand(inst->result);
                    if (!resultOpOpt) break;

                    auto resultOpLoc = findLocation(*resultOpOpt);
                    if (!resultOpLoc) break;

                    auto formattedResultLoc = formatLocation(*resultOpLoc);

                    if (auto arg1OpOpt = getNumOperand(inst->arg1)) {
                        emit(Operation::MOV, formattedResultLoc, *arg1OpOpt);
                        break;
                    }

                    auto arg1OpOpt = getOperand(inst->arg1);
                    if (!arg1OpOpt) throw std::runtime_error("NASM::generate: Unsupported operand type");

                    auto arg1Loc = findLocation(*arg1OpOpt);
                    if (!arg1Loc) throw std::runtime_error("NASM::generate: Arg1 operand not found");

                    auto formattedArg1Loc = formatLocation(*arg1Loc);
                    emitAloca(Operation::MOV, formattedResultLoc, formattedArg1Loc, resultOpLoc->kind == Location::STACK && arg1Loc->kind == Location::STACK);


                    if (auto arg2OpOpt = getNumOperand(inst->arg2)) {
                        emit(Operation::ADD, formattedResultLoc, *arg2OpOpt);
                        break;
                    }

                    auto arg2OpOpt = getOperand(inst->arg2);
                    if (!arg2OpOpt) throw std::runtime_error("NASM::generate: Unsupported operand type");

                    auto arg2Loc = findLocation(*arg2OpOpt);
                    if (!arg1Loc) throw std::runtime_error("NASM::generate: Arg2 operand not found");

                    auto formattedArg2Loc = formatLocation(*arg2Loc);
                    emitAloca(Operation::ADD, formattedResultLoc, formattedArg2Loc, resultOpLoc->kind == Location::STACK && arg2Loc->kind == Location::STACK);
                    break;
                }
                default:
                    throw std::runtime_error(fmt::format("NASM::generateFuncNasm: unsupported operation {}", typeid(inst->op).name()));
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

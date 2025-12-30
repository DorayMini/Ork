//
// Created by doray on 8/14/25.
//
#pragma once

#include <map>
#include <unordered_map>
#include <string>

#include "TACGenerator.h"


namespace ork::codeGenerator {
    struct StackFrame {
        int offset = 0;
    };

    struct Location {
        enum { REG, STACK } kind;
        std::string reg;
        int offset;

        static Location regLoc(std::string reg) {
            return Location {REG, std::move(reg), 0};
        }

        static Location stackLoc(const int off) {
            return Location {STACK, "", off};
        }
    };

    class NASM {
    public:
        [[nodiscard]] std::vector<std::string> generate(const std::vector<TACGenerator::Instruction> &insts);
    private:
        void generateFuncNasm(std::vector<TACGenerator::Instruction>::const_iterator begin, std::vector<TACGenerator::Instruction>::const_iterator end);

        static void liveIntervalsAnalysis(const std::vector<TACGenerator::Instruction> &insts);
        static std::optional<std::string> getOperand(const std::unique_ptr<TACGenerator::Operand> &a);
        static std::optional<int32_t> getNumOperand(const std::unique_ptr<TACGenerator::Operand> &a);
        static std::optional<std::string> allocateReg(const std::string &var, size_t index);
        static std::optional<Location> findLocation(const std::string &var);
        std::string formatLocation(const Location &loc);
        std::string formatStackInitialization();
        static int allocateStack(const std::string &var, int align, size_t index);

        static void freeReg(size_t index);

        static bool hasFreeRegister();

        inline static std::map<std::string, std::pair<int, int>> liveInterval{};

        inline static std::unordered_map<std::string, Location> varLocation{};


        inline static std::map<std::string, bool> variableRegs{
                {"eax", true},
                {"ebx", true},
                {"ecx", true},
                {"edx", true}
        };

        static void freeAllReg();

        inline static StackFrame stack{};

        std::vector<std::string> nasmCode = {
            "section .text\n", "global _start\n", "_start:\n",
            "    call main\n", "    mov eax, 1\n", "    mov ebx, 0\n",
            "    int 0x80\n"
        };

    };
} // namespace ork::codeGenerator

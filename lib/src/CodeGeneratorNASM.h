//
// Created by doray on 8/14/25.
//

#pragma once
#include <map>
#include <string>
#include <array>

#include "TACGenerator.h"

namespace ork::codeGenerator {

    struct Register {
        std::string name;
        std::optional<int> var_id = std::nullopt;
        int start = 0;
        int end = 0;
    };

    class NASM {
    public:
        void generate(const std::vector<TACGenerator::Instruction> &insts);
    private:
        inline static std::vector<Register> registers{
            {"eax"}, {"ebx"}, {"ecx"}, {"edx"}
        };
        std::optional<std::string> getVar(auto &&a);
        std::optional<std::string> getValue(auto &&a);
        static std::optional<Register*> allocateRegister();
        static std::optional<Register*> findRegisterByVarId(int var_id);

        void liveIntervalsAnalysis(const std::vector<TACGenerator::Instruction> &insts);

        std::map<std::string, std::pair<int, int>> liveInterval;

    };
} // namespace ork::codeGenerator

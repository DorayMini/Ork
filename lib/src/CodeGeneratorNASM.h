//
// Created by doray on 8/14/25.
//

#pragma once
#include <map>
#include <string>
#include <array>
#include <filesystem>

#include "TACGenerator.h"

namespace ork::codeGenerator {
    struct Register {
        std::string name;
        std::optional<int> var_id = std::nullopt; // live interval start
        int liveIntervalEnd = 0;
    };

    class NASM {
    public:
        void generate(const std::vector<TACGenerator::Instruction> &insts);

    private:
        inline static std::vector<Register> registers{
            {"eax"}, {"ebx"}, {"ecx"}, {"edx"}
        };

        static std::optional<std::string> getVar(const std::unique_ptr<TACGenerator::VarName> &a);

        static std::optional<std::string> getValue(const std::unique_ptr<TACGenerator::VarName> &a);

        static std::optional<Register *> allocateRegister();

        static std::optional<Register *> findRegisterByVarId(int var_id);

        std::variant<Register *, std::string> resolveOperand(const std::unique_ptr<TACGenerator::VarName> &arg);

        Register *resolveByArg(const std::string &result_var /* getVar() string */,
                               std::variant<Register *, std::string /* value */> arg);

        void liveIntervalsAnalysis(const std::vector<TACGenerator::Instruction> &insts);

        std::filesystem::path path;
        std::map<std::string, std::pair<int, int> > liveInterval;
    };
} // namespace ork::codeGenerator

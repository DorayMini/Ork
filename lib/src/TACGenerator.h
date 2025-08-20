//
// Created by doray on 8/13/25.
//

#pragma once
#include <unordered_set>
#include <vector>

#include "expression.h"
#include "fmt/chrono.h"

namespace ork::TACGenerator {
    enum class Operation {
        ADD,
        SUB,
        MULT,
        DIV,
        ALLOCA,
        FUNC_START,
        FUNC_END,
    };

    using VarName = std::variant<std::string, Value>;

    struct Instruction {
        Operation op;
        std::unique_ptr<VarName> arg1 = nullptr;
        std::unique_ptr<VarName> arg2 = nullptr;
        std::unique_ptr<VarName> result;

        bool operator==(const Instruction &other) const;
    };

    class Generator {
    public:
        void proccess(std::vector<std::unique_ptr<expression::Base>> nodes);
        std::vector<Instruction> takeInstructions();

    private:
        std::unique_ptr<VarName> generate(std::unique_ptr<expression::Base> node);
        static Operation getOperation(const expression::BinaryOp &node);
        static std::optional<std::unique_ptr<VarName>> foldVariable(const Operation& op, const VarName& val1, const VarName& val2);

        std::vector<Instruction> instructions;
        unsigned int counter = 0;
        std::unordered_set<std::string> functionNames;
    };
} // namespace ork::TACGenerator

//
// Created by doray on 8/13/25.
//

#pragma once
#include <vector>

#include "expression.h"
#include "fmt/chrono.h"
#include "tool.h"

namespace TACGenerator {
    enum class Operation {
        ADD,
        SUB,
        MULT,
        DIV,
        ALLOCA
    };

    using VarName = std::variant<expression::Identifier, std::string, Value>;

    struct Instruction {
        Operation op;
        std::unique_ptr<VarName> arg1;
        std::unique_ptr<VarName> arg2 = nullptr;
        std::unique_ptr<VarName> result;

        bool operator==(const Instruction &other) const;
    };

    class Generator {
    public:
        std::unique_ptr<VarName> generate(std::unique_ptr<expression::Base> node);

        std::vector<Instruction> takeInstructions();

    private:
        static Operation getOperation(const expression::BinaryOp &node);

        std::vector<Instruction> instructions;
        unsigned int counter = 0;
    };
} // namespace TACGenerator

//
// Created by doray on 8/14/25.
//
#pragma once

#include <map>
#include <TACGenerator.h>

using namespace ork::TACGenerator;

namespace ork::codeGenerator::instructionSelectionTable {
    inline std::map<Operation, std::string> NASM_REG{
        {Operation::ALLOCA, "mov {}, {}\n"},
        {Operation::ADD, "add {}, {}\n"},
        {Operation::SUB, "sub {}, {}\n"},
        {Operation::MULT, "imul {}, {}\n"},
        {Operation::DIV, "idiv {}, {}\n"},
    };
} // namespace ork::codeGenerator

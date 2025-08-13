//
// Created by doray on 8/13/25.
//

#include "TACGenerator.h"

#include <variant>

#include <typeinfo>
#include <cxxabi.h>

std::string dynamic_type(expression::Base* obj) {
    int status;
    const char* mangled = typeid(*obj).name();
    char* demangled = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
    std::string r = status == 0 ? demangled : mangled;
    std::free(demangled);
    return r;
}

std::unique_ptr<TACGenerator::VarName> TACGenerator::Generator::generate(std::unique_ptr<expression::Base> node) {
    if (auto i = dynamic_cast<expression::Identifier *>(node.get())) {
        return std::make_unique<VarName>(*i);
    }
    if (auto c = dynamic_cast<expression::Constant *>(node.get())) {
        if (std::holds_alternative<int32_t>(c->value)) {
            return std::make_unique<VarName>(
                fmt::to_string(std::get<int32_t>(c->value)));
        }
    }

    if (auto b = dynamic_cast<expression::Binary *>(node.get())) {
        std::string result = fmt::format("f{}", counter++);
        instructions.push_back(std::move(Instruction{
            .op = getOperation(b->op),
            .arg1 = std::move(generate(std::move(b->lhs))),
            .arg2 = generate(std::move(b->rhs)),
            .result = std::make_unique<VarName>(result)
        }));
        fmt::println("{}", result);
        return std::make_unique<VarName>(result);
    }
    if (auto var = dynamic_cast<expression::Variable *>(node.get())) {
        instructions.push_back(std::move(Instruction{
            .op = Operation::ALLOCA,
            .arg1 = generate(std::move(var->value)),
            .result = std::make_unique<VarName>(*var->name)
        }));

        return std::make_unique<VarName>(*var->name);
    }

    throw std::runtime_error(fmt::format("TACGenerator -> Unknown node type {}", typeid(*node).name()));
}

std::vector<TACGenerator::Instruction> TACGenerator::Generator::takeInstructions() {
    return std::move(instructions);
}

TACGenerator::Operation TACGenerator::Generator::getOperation(const expression::BinaryOp& node) {
    switch (node) {
        case expression::BinaryOp::Plus:
            return Operation::ADD;
        case expression::BinaryOp::Minus:
            return Operation::SUB;
        case expression::BinaryOp::Asterisk:
            return Operation::MULT;
        case expression::BinaryOp::Slash:
            return Operation::DIV;
        default:
            throw std::runtime_error("TACgenerator -> Unknown operation type");
    }
}


#include "Sema.h"

void ork::Sema::check(std::vector<std::unique_ptr<expression::Base>> &nodes) {
    for (auto& node : nodes) {
        visit(node);
    }

    if (!error_.empty()) {
        throw std::runtime_error(error_);
    }
}

void ork::Sema::visit(std::unique_ptr<expression::Base>& node) {
    if (auto func = dynamic_cast<expression::FunctionDecl *>(node.get())) {
        for (auto& exp: func->body) {
            visit(exp);
        }
    }
    else if (const auto var = dynamic_cast<expression::Variable *>(node.get())) {
        if (symbolTable_.contains(var->name->name)) {
            error_ += "\nVariable " + var->name->name + " is already defined";
            return;
        }
        symbolTable_[var->name->name] = Symbol {.type = var->type};
        visit(var->value);

        expression::Type actual = inferType(var->value);
        if (actual != var->type) {
            error_ += "\nCannot assign value of type " + expression::toString(actual) +
                    " to variable of type " + expression::toString(var->type);
        }
    }
    else if (const auto bin = dynamic_cast<expression::Binary *>(node.get())) {
        visit(bin->lhs);
        visit(bin->rhs);
    }
    else if (const auto id = dynamic_cast<expression::Identifier *>(node.get())) {
        if (!symbolTable_.contains(id->name)) {
            error_ += "\nVariable " + id->name + " is not defined";
        }
    }
}

ork::expression::Type ork::Sema::inferType(const std::unique_ptr<expression::Base> &node) {
    if (const auto con = dynamic_cast<expression::Constant *>(node.get())) {
        return con->type;
    }

    if (const auto id = dynamic_cast<expression::Identifier *>(node.get())) {
        if (symbolTable_.contains(id->name)) {
            return symbolTable_.find(id->name)->second.type;
        }
    }

    if (const auto bin = dynamic_cast<expression::Binary *>(node.get())) {
        auto lhs = inferType(bin->lhs);
        auto rhs = inferType(bin->rhs);

        switch (bin->op) {
            case expression::BinaryOp::Minus:
            case expression::BinaryOp::Plus:
            case expression::BinaryOp::Asterisk:
            case expression::BinaryOp::Slash:
            case expression::BinaryOp::Equal:
                if (lhs != expression::Type::Int32 && rhs != expression::Type::Int32)
                    error_ += "\nOperands must be integers";
                return expression::Type::Int32;
            case expression::BinaryOp::Greater:
            case expression::BinaryOp::Less:
            case expression::BinaryOp::GreaterEqual:
            case expression::BinaryOp::LessEqual:
            case expression::BinaryOp::EqualEqual:
                if (lhs != rhs)
                    error_ += "\nOperands must have same type";
                return expression::Type::Bool;
            case expression::BinaryOp::OrOr:
            case expression::BinaryOp::AndAnd:
                if (lhs != expression::Type::Bool && rhs != expression::Type::Bool)
                    error_ += "\nOperands must be boolean";
                return expression::Type::Bool;
            default:
                throw std::runtime_error("Unknown binary operator");
        }
    }
    throw std::runtime_error("Cannot infer type for this node");
}



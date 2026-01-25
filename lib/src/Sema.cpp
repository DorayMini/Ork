
#include "Sema.h"

void ork::Sema::check(std::vector<std::unique_ptr<expression::Base>> &nodes) {
    for (auto& node : nodes) {
        visit(node);
    }

    if (!_error.empty()) {
        throw std::runtime_error(_error);
    }
}

void ork::Sema::visit(std::unique_ptr<expression::Base>& node) {
    if (auto func = dynamic_cast<expression::FunctionDecl *>(node.get())) {
        for (auto& exp: func->body) {
            visit(exp);
        }
    }
    if (const auto var = dynamic_cast<expression::Variable *>(node.get())) {
        if (_symbolTable.contains(var->name->name)) {
            _error += "\nVariable " + var->name->name + " is already defined";
            return;
        }
        _symbolTable.emplace(var->name->name);
        visit(var->value);
    }
    else if (const auto bin = dynamic_cast<expression::Binary *>(node.get())) {
        visit(bin->lhs);
        visit(bin->rhs);
    }
    else if (const auto id = dynamic_cast<expression::Identifier *>(node.get())) {
        if (!_symbolTable.contains(id->name)) {
            _error += "\nVariable " + id->name + " is not defined";
        }
    }
}

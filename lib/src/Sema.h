#pragma once
#include <stack>
#include <string>
#include <unordered_map>
#include "parser.h"

#include "token.h"

namespace ork {

class Sema {
public:
    Sema() = default;
    void check(std::vector<std::unique_ptr<expression::Base>>& nodes);

private:
    struct Symbol {
        expression::Type type;
    };

    std::string error_;
    std::unordered_map<std::string, Symbol> symbolTable_;

    expression::Type inferType(const std::unique_ptr<expression::Base>& node);

    void visit(std::unique_ptr<expression::Base>& node);
};

} // ork

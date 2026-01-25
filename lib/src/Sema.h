#pragma once
#include <stack>
#include <string>
#include <unordered_set>
#include "parser.h"

#include "token.h"

namespace ork {

class Sema {
public:
    Sema() = default;
    void check(std::vector<std::unique_ptr<expression::Base>>& nodes);
    void visit(std::unique_ptr<expression::Base>& node);

private:
    std::string _error = "";
    std::unordered_set<std::string> _symbolTable;
};

} // ork

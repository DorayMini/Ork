#pragma once
#include <string_view>
#include <vector>

#include "token.h"

namespace lexer {
    std::vector<token::Lexem> proccess(std::string_view input);;
} // namespace lexer::token

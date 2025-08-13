//
// Created by doray on 8/11/25.
//

#pragma once

#include "expression.h"
#include <span>

#include "token.h"

class parser {
public:
     explicit parser(std::span<lexer::token::Lexem> tokens) : tokens_(tokens) {}

     std::unique_ptr<expression::Base> parse();
private:
     std::span<lexer::token::Lexem> tokens_;

     std::unique_ptr<expression::Base> parseExpression(int leftBindingPower = 0);
     std::unique_ptr<expression::Variable> parseVariable();

     [[nodiscard]] const lexer::token::Lexem& peek() const {
          return tokens_.front();
     }

     lexer::token::Lexem take() {
          auto token = tokens_.front();
          tokens_ = tokens_.subspan(1);
          return token;
     }
};



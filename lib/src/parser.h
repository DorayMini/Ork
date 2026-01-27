//
// Created by doray on 8/11/25.
//

#pragma once

#include <map>

#include "expression.h"
#include <span>

#include "token.h"

namespace ork {
     class parser {
     public:
          explicit parser(std::span<lexer::token::Lexem> tokens) : tokens_(tokens) {}

          std::vector<std::unique_ptr<expression::Base>> parse();

     private:
          std::span<lexer::token::Lexem> tokens_;

          inline static std::map<lexer::token::Lexem, std::tuple<int, expression::BinaryOp>> priority_{
               // Arithmetic
               {lexer::token::PLUS{}, std::make_tuple(1, expression::BinaryOp::Plus)},
               {lexer::token::MINUS{}, {1, expression::BinaryOp::Minus}},
               {lexer::token::ASTERISK{}, {2, expression::BinaryOp::Asterisk}},
               {lexer::token::SLASH{}, {2, expression::BinaryOp::Slash}},

               // Comparison
               {lexer::token::EQUAL_EQUAL{}, {3, expression::BinaryOp::Equal}},
               {lexer::token::LESS{}, {4, expression::BinaryOp::Less}},
               {lexer::token::LESS_EQUAL{}, {4, expression::BinaryOp::LessEqual}},
               {lexer::token::GREATER{}, {4, expression::BinaryOp::Greater}},
               {lexer::token::GREATER_EQUAL{}, {4, expression::BinaryOp::GreaterEqual}},

               // Logical
               {lexer::token::OR_OR{}, {1, expression::BinaryOp::OrOr}},
               {lexer::token::AND_AND{}, {2, expression::BinaryOp::AndAnd}},
          };


          std::unique_ptr<expression::Base> parseStatement();
          void parseCodeBlock(std::vector<std::unique_ptr<expression::Base>>& output);

          bool isAtStatementTerminator(int leftBindingPower) const;
          std::unique_ptr<expression::Base> parseExpression(int leftBindingPower = 0);
          std::unique_ptr<expression::Variable> parseVariable();
          std::unique_ptr<expression::FunctionDecl> parseFunctionDecl();
          std::unique_ptr<expression::Base> parseIfStatement();


          [[nodiscard]] const lexer::token::Lexem& peek() const {
               return tokens_.front();
          }

          lexer::token::Lexem take() {
               auto token = tokens_.front();
               tokens_ = tokens_.subspan(1);
               return token;
          }
     };
} // namespace ork;



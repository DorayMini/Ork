//
// Created by doray on 8/11/25.
//
#include "tool.h"

#include "parser.h"

#include <variant>

#include "fmt/format.h"
#include "expression.h"

std::unique_ptr<expression::Base> parser::parse() {
    if (std::holds_alternative<lexer::token::IDENTIFIER>(peek())) {
        return parseVariable();
    }
    return parseExpression();
}

std::unique_ptr<expression::Base> parser::parseExpression(int leftBindingPower) {
    auto lhs = std::visit(
        match{
            [](lexer::token::INTEGER token) -> std::unique_ptr<expression::Base> {
                return std::make_unique<expression::Constant>(expression::Constant(token.value));
            },
            [](auto &&) -> std::unique_ptr<expression::Base> {
                throw std::runtime_error(fmt::format("Unexpected token. Expected binary operator"));
            }
        },
        take());

    if (tokens_.empty()) {
        return lhs;
    }
    if (std::holds_alternative<lexer::token::SEMICOLON>(peek())) {
        return lhs;
    }

    for (;;) {
        auto [rightBindingPower, op] = std::visit(
            match{
                [](lexer::token::PLUS token) -> std::tuple<int, expression::Op> {
                    return {1, token};
                },
                [](lexer::token::MINUS token) -> std::tuple<int, expression::Op> {
                    return {1, token};
                },
                [](lexer::token::ASTERISK token) -> std::tuple<int, expression::Op> {
                    return {2, token};
                },
                [](lexer::token::SLASH token) -> std::tuple<int, expression::Op> {
                    return {2, token};
                },
                [](auto &&) -> std::tuple<int, expression::Op> {
                    throw std::runtime_error(fmt::format("TODO: error"));
                }
            },
            peek());


        if (rightBindingPower < leftBindingPower) {
            return lhs;
        }
        take();
        auto rhs = parseExpression(rightBindingPower);

        lhs = std::make_unique<expression::Binary>(op, std::move(lhs), std::move(rhs));

        if (tokens_.empty()) {
            break;
        }
        if (std::holds_alternative<lexer::token::SEMICOLON>(peek())) {
            break;
        }
    }

    return lhs;
}

std::unique_ptr<expression::Base> parser::parseVariable() {
    auto lhs = std::visit(
        match{
            [] (lexer::token::IDENTIFIER token) -> std::unique_ptr<expression::Base> {
                return std::make_unique<expression::Identifier>(fmt::to_string(token.value));
            },
            [] (auto&&) -> std::unique_ptr<expression::Base> {
                throw std::runtime_error(fmt::format("Unexpected token."));
            }
        },
        take()
    );

    if (std::holds_alternative<lexer::token::EQUAL>(peek())) {
        take();
    } else {
        throw std::runtime_error(fmt::format("Unexpected token. Expected assignment operator '='"));
    }

    auto rhs = parseExpression();

    return std::make_unique<expression::Binary>(
        lexer::token::EQUAL{},
        std::move(lhs),
        std::move(rhs)
    );
}


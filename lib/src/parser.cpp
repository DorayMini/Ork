//
// Created by doray on 8/11/25.
//
#include "tool.h"

#include "parser.h"

#include <variant>

#include "fmt/format.h"
#include "expression.h"
#include "token.h"

namespace ork {
    std::vector<std::unique_ptr<expression::Base>> parser::parse() {
        std::vector<std::unique_ptr<expression::Base>> result;

        while (!tokens_.empty()) {
            result.emplace_back(std::move(parseCodeBlock()));
        }

        return result;
    }

    std::unique_ptr<expression::Base> parser::parseCodeBlock() {
        if (std::holds_alternative<lexer::token::KEYWORD>(peek())) {
            switch (std::get<lexer::token::KEYWORD>(take())) {
                case lexer::token::KEYWORD::FN:
                    return parseFunctionDecl();
                default:
                    throw std::runtime_error("unrecognized keyword");
            }
        }
        if (std::holds_alternative<lexer::token::TYPE>(peek())) {
            return parseVariable();
        }
        return parseExpression();
    }

    bool parser::isAtStatementTerminator(int leftBindingPower) const {
        return !tokens_.empty()
               && std::holds_alternative<lexer::token::SEMICOLON>(peek())
               && leftBindingPower == 0;
    }

    std::unique_ptr<expression::Base> parser::parseExpression(int leftBindingPower) {
        auto lhs = std::visit(
            match{
                [](lexer::token::INTEGER token) -> std::unique_ptr<expression::Base> {
                    return std::make_unique<expression::Constant>(expression::Constant(token.value));
                },
                [](auto &&) -> std::unique_ptr<expression::Base> {
                    throw std::runtime_error(fmt::format("Expected integer"));
                }
            },
            take());

        while (!(tokens_.empty() || std::holds_alternative<lexer::token::SEMICOLON>(peek()))) {
            auto [rightBindingPower, op] = std::visit(
                match{
                    [](lexer::token::PLUS) -> std::tuple<int, expression::BinaryOp> {
                        return {1, expression::BinaryOp::Plus};
                    },
                    [](lexer::token::MINUS) -> std::tuple<int, expression::BinaryOp> {
                        return {1, expression::BinaryOp::Minus};
                    },
                    [](lexer::token::ASTERISK) -> std::tuple<int, expression::BinaryOp> {
                        return {2, expression::BinaryOp::Asterisk};
                    },
                    [](lexer::token::SLASH) -> std::tuple<int, expression::BinaryOp> {
                        return {2, expression::BinaryOp::Slash};
                    },
                    []([[maybe_unused]] auto &&token) -> std::tuple<int, expression::BinaryOp> {
                        throw std::runtime_error(fmt::format("Unexpected token '{}'. Expected binary operator.",
                                                             typeid(token).name()));
                    }
                },
                peek());


            if (rightBindingPower < leftBindingPower) {
                return lhs;
            }

            take();
            auto rhs = parseExpression(rightBindingPower);

            lhs = std::make_unique<expression::Binary>(op, std::move(lhs), std::move(rhs));
        }


        if (isAtStatementTerminator(leftBindingPower)) {
            take();
        }

        return lhs;
    }

    std::unique_ptr<expression::Variable> parser::parseVariable() {
        auto type = std::visit(
            match{
                [](lexer::token::TYPE type) -> expression::VarType {
                    switch (type) {
                        case lexer::token::TYPE::INTEGER:
                            return expression::VarType::Int;
                        default:
                            throw std::runtime_error("unrecognized variable type");
                    }
                },
                [](auto &&) -> expression::VarType {
                    throw std::runtime_error("Unexpected token. Expected variable type");
                }
            },
            take()
        );
        auto lhs = std::visit(
            match{
                [](lexer::token::IDENTIFIER token) -> std::unique_ptr<expression::Identifier> {
                    return std::make_unique<expression::Identifier>(fmt::to_string(token.value));
                },
                [](auto &&) -> std::unique_ptr<expression::Identifier> {
                    throw std::runtime_error("Unexpected token.");
                }
            },
            take()
        );

        if (!std::holds_alternative<lexer::token::EQUAL>(take())) {
            throw std::runtime_error("Unexpected token. Expected assignment operator '='");
        }

        auto rhs = parseExpression();

        return std::make_unique<expression::Variable>(
            type,
            std::move(lhs),
            std::move(rhs)
        );
    }

    std::unique_ptr<expression::FunctionDecl> parser::parseFunctionDecl() {
        auto name = std::visit(
            match{
                [](lexer::token::IDENTIFIER id) -> std::unique_ptr<expression::Identifier> {
                    return std::make_unique<expression::Identifier>(fmt::to_string(id.value));
                },
                [](auto &&) -> std::unique_ptr<expression::Identifier> {
                    throw std::runtime_error("parseFunctionDecl: expected function name (identifier)");
                }
            },
            take());

        if (!std::holds_alternative<lexer::token::LPAR>(take())) {
            throw std::runtime_error("parseFunctionDecl: expected '(' after function name");
        }
        if (!std::holds_alternative<lexer::token::RPAR>(take())) {
            throw std::runtime_error("parseFunctionDecl: expected ')' after '('");
        }


        if (std::holds_alternative<lexer::token::LBRACE>(take())) {
            auto fun = std::make_unique<expression::FunctionDecl>(std::move(name));
            while (!std::holds_alternative<lexer::token::RBRACE>(peek())) {
                if (tokens_.empty()) {
                    throw std::runtime_error("parseFunctionDecl: unexpected end of tokens while parsing body");
                }

                auto expr = parseCodeBlock();

                if (!expr) {
                    throw std::runtime_error("parseFunctionDecl: failed to parse expression in function body");
                }

                fun->body.emplace_back(std::move(expr));
                if (!tokens_.empty() && std::holds_alternative<lexer::token::SEMICOLON>(peek())) {
                    take();
                }
            }

            take();
            return fun;
        } else {
            throw std::runtime_error("parseFunctionDecl: expected '{' to start function body");
        }
    }
} // namespace ork

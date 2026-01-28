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

        while (!_tokens.empty()) {
            result.emplace_back(std::move(parseStatement()));
        }

        return result;
    }

    std::unique_ptr<expression::Base> parser::parseStatement() {
        if (std::holds_alternative<lexer::token::KEYWORD>(peek())) {
            switch (std::get<lexer::token::KEYWORD>(peek())) {
                case lexer::token::KEYWORD::FN:
                    return parseFunctionDecl();
                case lexer::token::KEYWORD::IF:
                    return parseIfStatement();
                case lexer::token::KEYWORD::INT32:
                case lexer::token::KEYWORD::BOOL:
                    return parseVariable();
                default:
                    throw std::runtime_error("unrecognized keyword");
            }
        }
        return parseExpression();
    }

    void parser::parseCodeBlock(std::vector<std::unique_ptr<expression::Base>> &output) {
        while (!_tokens.empty() && !std::holds_alternative<lexer::token::RBRACE>(peek())) {
            output.emplace_back(parseStatement());
        }
    }

    bool parser::isAtStatementTerminator(int leftBindingPower) const {
        return !_tokens.empty()
               && std::holds_alternative<lexer::token::SEMICOLON>(peek())
               && leftBindingPower == 0;
    }

    std::unique_ptr<expression::Base> parser::parseExpression(int leftBindingPower) {
        auto lhs = std::visit(
            match{
                [](lexer::token::IDENTIFIER token) -> std::unique_ptr<expression::Base> {
                    return std::make_unique<expression::Identifier>(expression::Identifier(fmt::to_string(token.value)));
                },
                [](lexer::token::INTEGER token) -> std::unique_ptr<expression::Base> {
                    return std::make_unique<expression::Constant>(expression::Constant(expression::Type::Int32, token.value));
                },
                [this](lexer::token::LPAR token) -> std::unique_ptr<expression::Base> {
                    auto expr = parseExpression(0);
                    if (!std::holds_alternative<lexer::token::RPAR>(peek())) {
                        throw std::runtime_error("unrecognized rpar");
                    }
                    take();
                    return expr;
                },
                [] (lexer::token::KEYWORD key) -> std::unique_ptr<expression::Base> {
                    switch (key) {
                        case lexer::token::KEYWORD::TRUE:
                            return std::make_unique<expression::Constant>(expression::Type::Bool, true);
                        case lexer::token::KEYWORD::FALSE:
                            return std::make_unique<expression::Constant>(expression::Type::Bool, false);
                        default:
                            throw std::runtime_error("Expected boolean literal 'true' or 'false'");
                    }
                },
                [](auto &&) -> std::unique_ptr<expression::Base> {
                    throw std::runtime_error(fmt::format("Expected integer"));
                }
            },
            take());

        while (!(_tokens.empty() || std::holds_alternative<lexer::token::SEMICOLON>(peek()))) {
            auto token = peek();

            auto it = _priority.find(token);
            if (it == _priority.end()) {
                return lhs;
            }
            auto [rightBindingPower, op] = it->second;


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
                [](lexer::token::KEYWORD type) -> expression::Type {
                    if (auto it = _types.find(type); it != _types.end())
                        return it->second;
                    throw std::runtime_error("Unexpected token. Expected variable type");
                },
                [](auto && type) -> expression::Type {
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
        take(); // take "fn"

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

        if (!std::holds_alternative<lexer::token::LBRACE>(take()))
            throw std::runtime_error("parseFunctionDecl: expected '{' to start function body");

        auto fun = std::make_unique<expression::FunctionDecl>(std::move(name));
        parseCodeBlock(fun->body);

        if (!std::holds_alternative<lexer::token::RBRACE>(take())) {
            throw std::runtime_error("parseFunctionDecl: expected '}'");
        }
        return fun;

    }

    std::unique_ptr<expression::Base> parser::parseIfStatement() {
        take(); // take "if"
        auto If = std::make_unique<expression::IfStatement>(std::move(parseExpression(0)));

        if (!std::holds_alternative<lexer::token::LBRACE>(take())) {
            throw std::runtime_error("parseIfStatement: expected '{'");
        }

        parseCodeBlock(If->then);

        if (!std::holds_alternative<lexer::token::RBRACE>(take())) {
            throw std::runtime_error("parseIfStatement: expected '}'");
        }

        return If;
    }
} // namespace ork

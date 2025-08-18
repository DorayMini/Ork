//
// Created by doray on 8/11/25.
//

#include <gtest/gtest.h>
#include <span>

#include "tokenizer.h"
#include "parser.h"

using namespace ork;
TEST(Parser, Case1) {
    std::vector t(lexer::proccess("2 * 2 + 2;"));
    parser p{std::span(t)};

    expression::Binary expected(
        expression::BinaryOp::Plus,
        std::make_unique<expression::Binary>(
            expression::BinaryOp::Asterisk,
            std::make_unique<expression::Constant>(2),
            std::make_unique<expression::Constant>(2)
        ),
        std::make_unique<expression::Constant>(2)
    );

    auto parsed = p.parse();
    bool equal = parsed->equals(expected);
    EXPECT_TRUE(equal);
}

TEST(Parser, Case2) {
    std::vector t(lexer::proccess("2 + 2 * 2;"));
    parser p{std::span(t)};
    expression::Binary expected(
        expression::BinaryOp::Plus,
        std::make_unique<expression::Constant>(2),
        std::make_unique<expression::Binary>(
            expression::BinaryOp::Asterisk,
            std::make_unique<expression::Constant>(2),
            std::make_unique<expression::Constant>(2)
        )
    );

    auto parsed = p.parse();
    bool equal = parsed->equals(expected);
    EXPECT_TRUE(equal);
}

TEST(Parser, Case3) {
    std::vector t(lexer::proccess("int a = 2;"));
    parser p{std::span(t)};
    expression::Variable expected(
        expression::VarType::Int,
        std::make_unique<expression::Identifier>("a"),
        std::make_unique<expression::Constant>(2)
    );

    auto parsed = p.parse();
    bool equal = parsed->equals(expected);
    EXPECT_TRUE(equal);
}

TEST(Parser, Case4) {
    std::vector t(lexer::proccess("int a = 2 + 2;"));
    parser p{std::span(t)};
    expression::Variable expected(
        expression::VarType::Int,
        std::make_unique<expression::Identifier>("a"),
        std::make_unique<expression::Binary>(
            expression::BinaryOp::Plus,
            std::make_unique<expression::Constant>(2),
            std::make_unique<expression::Constant>(2)
        )
    );

    auto parsed = p.parse();
    bool equal = parsed->equals(expected);
    EXPECT_TRUE(equal);
}

TEST(Parser, FUNC_CASE1) {
    std::vector t(lexer::proccess("fn main() { int a = 2; int b = 4;}"));
    parser p{std::span(t)};
    std::vector<std::unique_ptr<expression::Base>> body;

    body.push_back(std::make_unique<expression::Variable>(
        expression::VarType::Int,
        std::make_unique<expression::Identifier>("a"),
        std::make_unique<expression::Constant>(2)
    ));

    expression::FunctionDecl expected(
        std::make_unique<expression::Identifier>("main"),
        std::move(body)
    );

    auto parsed = p.parse();
    bool equal = parsed->equals(expected);
    EXPECT_TRUE(equal);
}

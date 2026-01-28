//
// Created by doray on 8/11/25.
//

#include <gtest/gtest.h>
#include <span>

#include "tokenizer.h"
#include "parser.h"

using namespace ork;
TEST(Parser, Case1) {
    std::vector t(lexer::proccess("2 * 2 + 2"));
    parser p{std::span(t)};

    expression::Binary expected(
        expression::BinaryOp::Plus,
        std::make_unique<expression::Binary>(
            expression::BinaryOp::Asterisk,
            std::make_unique<expression::Constant>(expression::Type::Int32, 2),
            std::make_unique<expression::Constant>(expression::Type::Int32, 2)
        ),
        std::make_unique<expression::Constant>(expression::Type::Int32, 2)
    );

    auto parsed = p.parse();
    bool equal = parsed[0]->equals(expected);
    EXPECT_TRUE(equal);
}

TEST(Parser, Case2) {
    std::vector t(lexer::proccess("(2 + 2) * 2;"));
    parser p{std::span(t)};

    expression::Binary expected(
        expression::BinaryOp::Asterisk,
        std::make_unique<expression::Binary>(
            expression::BinaryOp::Plus,
            std::make_unique<expression::Constant>(expression::Type::Int32, 2),
            std::make_unique<expression::Constant>(expression::Type::Int32, 2)
        ),
        std::make_unique<expression::Constant>(expression::Type::Int32, 2)
    );

    auto parsed = p.parse();


    bool equal = parsed[0]->equals(expected);
    EXPECT_TRUE(equal);
}

TEST(Parser, Case3) {
    std::vector t(lexer::proccess("i32 a = 2;"));
    parser p{std::span(t)};
    expression::Variable expected(
        expression::Type::Int32,
        std::make_unique<expression::Identifier>("a"),
        std::make_unique<expression::Constant>(expression::Type::Int32, 2)
    );

    auto parsed = p.parse();
    bool equal = parsed[0]->equals(expected);
    EXPECT_TRUE(equal);
}

TEST(Parser, Case4) {
    std::vector t(lexer::proccess("i32 a = 2 + 2;"));
    parser p{std::span(t)};
    expression::Variable expected(
        expression::Type::Int32,
        std::make_unique<expression::Identifier>("a"),
        std::make_unique<expression::Binary>(
            expression::BinaryOp::Plus,
            std::make_unique<expression::Constant>(expression::Type::Int32, 2),
            std::make_unique<expression::Constant>(expression::Type::Int32, 2)
        )
    );

    auto parsed = p.parse();

    bool equal = parsed[0]->equals(expected);
    EXPECT_TRUE(equal);
}

TEST(Parser, FUNC_CASE1) {
    std::vector t(lexer::proccess("fn main() { i32 a = 2; i32 b = 4;}"));
    parser p{std::span(t)};
    std::vector<std::unique_ptr<expression::Base>> body;

    body.push_back(std::make_unique<expression::Variable>(
        expression::Type::Int32,
        std::make_unique<expression::Identifier>("a"),
        std::make_unique<expression::Constant>(expression::Type::Int32, 2)
    ));

    body.push_back(std::make_unique<expression::Variable>(
         expression::Type::Int32,
        std::make_unique<expression::Identifier>("b"),
        std::make_unique<expression::Constant>(expression::Type::Int32, 4)
    ));

    expression::FunctionDecl expected(
        std::make_unique<expression::Identifier>("main"),
        std::move(body)
    );

    auto parsed = p.parse();
    bool equal = parsed[0]->equals(expected);
    EXPECT_TRUE(equal);
}

TEST(Parser, LogicalOperatorsPriority) {
    std::vector t(lexer::proccess("1 && 2 || 3;"));
    parser p{std::span(t)};

    auto left_and = std::make_unique<expression::Binary>(
        expression::BinaryOp::AndAnd,
        std::make_unique<expression::Constant>(expression::Type::Int32, 1),
        std::make_unique<expression::Constant>(expression::Type::Int32, 2)
    );

    expression::Binary expected(
        expression::BinaryOp::OrOr,
        std::move(left_and),
        std::make_unique<expression::Constant>(expression::Type::Int32, 3)
    );

    auto parsed = p.parse();

    ASSERT_FALSE(parsed.empty());
    bool equal = parsed[0]->equals(expected);
    EXPECT_TRUE(equal);
}

TEST(Parser, IfElseStatement) {
    std::vector t(lexer::proccess("if a > 5 {10;}"));
    parser p{std::span(t)};

    auto condition = std::make_unique<expression::Binary>(
        expression::BinaryOp::Greater,
        std::make_unique<expression::Identifier>("a"),
        std::make_unique<expression::Constant>(expression::Type::Int32, 5)
    );

    auto then_branch = std::vector<std::unique_ptr<expression::Base>>();
    then_branch.push_back(std::make_unique<expression::Constant>(expression::Type::Int32, 10));

    auto expected_if = std::make_unique<expression::IfStatement>(
        std::move(condition),
        std::move(then_branch)
    );

    auto parsed = p.parse();

    ASSERT_FALSE(parsed.empty());
    EXPECT_TRUE(parsed[0]->equals(*expected_if));
}


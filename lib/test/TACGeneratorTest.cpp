//
// Created by doray on 8/14/25.
//

#include "parser.h"
#include "gtest/gtest.h"
#include "TACGenerator.h"
#include "tokenizer.h"

using namespace ork;

TEST(TECGenerator, Case1) {
    std::vector t(lexer::proccess("int a = 2 + 2;"));
    parser p{std::span(t)};
    TACGenerator::Generator g;
    g.generate(p.parse());
    auto instr = g.takeInstructions();

    std::vector<TACGenerator::Instruction> expected;
    expected.emplace_back(
        TACGenerator::Instruction{
            .op = TACGenerator::Operation::ALLOCA,
            .arg1 = std::make_unique<TACGenerator::VarName>(4),
            .result = std::make_unique<TACGenerator::VarName>(expression::Identifier("a"))
        }
    );

    EXPECT_EQ(instr, expected);
}

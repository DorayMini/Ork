//
// Created by doray on 8/14/25.
//

#include "parser.h"
#include "gtest/gtest.h"
#include "TACGenerator.h"
#include "tokenizer.h"

using namespace ork;

TEST(TECGenerator, Case1) {
    std::vector t(lexer::proccess("fn main() {int a = 2;}"));
    parser p{std::span(t)};
    TACGenerator::Generator g;


    g.proccess(std::move(p.parse()));
    auto instr = g.takeInstructions();

    std::vector<TACGenerator::Instruction> expected;
    expected.emplace_back(
        TACGenerator::Instruction{
            .op = TACGenerator::Operation::FUNC_START,
            .result = std::make_unique<TACGenerator::Operand>("main")
        }
    );

    expected.emplace_back(
        TACGenerator::Instruction{
            .op = TACGenerator::Operation::MOV,
            .arg1 = std::make_unique<TACGenerator::Operand>(2),
            .result = std::make_unique<TACGenerator::Operand>("a")
        }
    );

    expected.emplace_back(
        TACGenerator::Instruction{
            .op = TACGenerator::Operation::FUNC_END,
            .result = std::make_unique<TACGenerator::Operand>("main")
        }
    );

    EXPECT_EQ(instr, expected);
}

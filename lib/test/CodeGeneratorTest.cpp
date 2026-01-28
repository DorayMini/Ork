//
// Created by doray on 8/14/25.
//

#include "gtest/gtest.h"
#include "parser.h"
#include "TACGenerator.h"
#include "tokenizer.h"
#include "Sema.h"
#include "CodeGeneratorNASM.h"

using namespace ork;
TEST(CodeGenerate, Case1) {
    std::vector t(lexer::proccess("fn main() {i32 d = 3;}"));
    parser p{std::span(t)};

    auto parsed = p.parse();

    Sema sema;
    sema.check(parsed);

    TACGenerator::Generator g;
    g.proccess(std::move(parsed));
    auto instr = g.takeInstructions();

    codeGenerator::NASM nasm;
    auto _ = nasm.generate(instr);

    EXPECT_TRUE(true);
}

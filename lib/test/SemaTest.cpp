#include "parser.h"
#include "TACGenerator.h"
#include "tokenizer.h"
#include "gtest/gtest.h"
#include "Sema.h"

using namespace ork;
TEST(Sema, Case1) {
    std::vector t(lexer::proccess("fn main() {i32 d = 2; bool b = 2 > 1;}"));
    parser p{std::span(t)};

    auto parsed = p.parse();

    Sema sema;

    sema.check(parsed);

    EXPECT_THROW(
        sema.check(parsed),
        std::runtime_error
    );
}

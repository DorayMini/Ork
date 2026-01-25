#include "parser.h"
#include "TACGenerator.h"
#include "tokenizer.h"
#include "gtest/gtest.h"
#include "Sema.h"

using namespace ork;
TEST(Sema, Case1) {
    std::vector t(lexer::proccess("fn main() {int d = 2; int b = a;}"));
    parser p{std::span(t)};

    auto parsed = p.parse();

    Sema sema;

    EXPECT_THROW(
        sema.check(parsed),
        std::runtime_error
    );
}

#include <gtest/gtest.h>
#include "tokenizer.h"

using namespace ork;
using namespace lexer::token;
TEST(Lexer, Case1) {

  std::string_view source = "fn main() { return 42; }";

  std::vector<Lexem> processed = lexer::proccess(source);

  std::vector<Lexem> expected = {
    KEYWORD::FN,
    IDENTIFIER{"main"},
    LPAR{},
    RPAR{},
    LBRACE{},
    KEYWORD::RETURN,
    INTEGER{42},
    SEMICOLON{},
    RBRACE{}
  };

  EXPECT_EQ(processed, expected);
}

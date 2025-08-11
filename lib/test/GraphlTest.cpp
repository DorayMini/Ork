#include <gtest/gtest.h>
#include "tokenizer.h"

using namespace lexer::token;
TEST(Lexer, Case1) {

  std::string_view source = "fn main() { return 42; }";

  std::vector<Token> processed = lexer::proccess(source);

  std::vector<Token> expected = {
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

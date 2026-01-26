#include <gtest/gtest.h>
#include "tokenizer.h"

using namespace ork;
using namespace lexer::token;
TEST(Lexer, Case1) {

  std::string_view source = "fn main() { == return 42; }";

  std::vector<Lexem> processed = lexer::proccess(source);

  std::vector<Lexem> expected = {
    KEYWORD::FN,
    IDENTIFIER{"main"},
    LPAR{},
    RPAR{},
    LBRACE{},
    EQUAL_EQUAL{},
    KEYWORD::RETURN,
    INTEGER{42},
    SEMICOLON{},
    RBRACE{}
  };

  EXPECT_EQ(processed, expected);
}

TEST(Lexer, Case2) {
  std::string_view source = "fn main() { if (2 == 3 && 2 < 3 || 3 > 2) {} }";

  std::vector<Lexem> processed = lexer::proccess(source);

  std::vector<Lexem> expected = {
    KEYWORD::FN,
    IDENTIFIER{"main"},
    LPAR{},
    RPAR{},
    LBRACE{},

    KEYWORD::IF,
    LPAR{},
    INTEGER{2},
    EQUAL_EQUAL{},
    INTEGER{3},
    AND_AND{},
    INTEGER{2},
    LESS{},
    INTEGER{3},
    OR_OR{},
    INTEGER{3},
    GREATER{},
    INTEGER{2},
    RPAR{},

    LBRACE{},
    RBRACE{},

    RBRACE{}
  };

  EXPECT_EQ(processed, expected);

}

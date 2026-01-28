#pragma once
#include <string>
#include <variant>

#include "value.h"

namespace ork::lexer::token {
    enum class KEYWORD {
        FN,
        IF,
        RETURN,
        INT32,
        BOOL,
        TRUE,
        FALSE
    };

    struct SEMICOLON { auto operator<=>(const SEMICOLON &) const = default; }; // ;
    struct LPAR   { auto operator<=>(const LPAR &) const = default;   }; // (
    struct RPAR   { auto operator<=>(const RPAR &) const = default;   }; // )
    struct LBRACE { auto operator<=>(const LBRACE &) const = default; }; // {
    struct RBRACE { auto operator<=>(const RBRACE &) const = default; }; // }
    struct COMMA  { auto operator<=>(const COMMA &) const = default;  }; // ,
    struct PLUS { auto operator<=>(const PLUS &) const = default; }; // +
    struct MINUS { auto operator<=>(const MINUS &) const = default; }; // -
    struct ASTERISK { auto operator<=>(const ASTERISK &) const = default; }; // *
    struct SLASH { auto operator<=>(const SLASH &) const = default; }; // /
    struct GREATER { auto operator<=>(const GREATER &) const = default; }; // >
    struct LESS { auto operator<=>(const LESS &) const = default; }; // <
    struct GREATER_EQUAL { auto operator<=>(const GREATER_EQUAL &) const = default; }; // >=
    struct LESS_EQUAL { auto operator<=>(const LESS_EQUAL &) const = default; }; // <=
    struct EQUAL { auto operator<=>(const EQUAL &) const = default; }; // =
    struct OR_OR { auto operator<=>(const OR_OR &) const = default; }; // ||
    struct AND_AND { auto operator<=>(const AND_AND &) const = default; }; // &&
    struct EQUAL_EQUAL { auto operator<=>(const EQUAL_EQUAL &) const = default; }; // ==
    struct IDENTIFIER {
        auto operator<=>(const IDENTIFIER &) const = default;
        std::string_view value;
    };
    struct INTEGER {
        auto operator<=>(const INTEGER &) const = default;
        int value;
    };
    struct STRING {
        auto operator<=>(const STRING &) const = default;
        std::string_view value;
    };

    using Lexem = std::variant<
        KEYWORD,
        LPAR,
        RPAR,
        LBRACE,
        RBRACE,
        IDENTIFIER,
        SEMICOLON,
        COMMA,
        INTEGER,
        STRING,
        PLUS,
        MINUS,
        ASTERISK,
        SLASH,
        GREATER,
        LESS,
        GREATER_EQUAL,
        LESS_EQUAL,
        EQUAL,
        OR_OR,
        AND_AND,
        EQUAL_EQUAL
    >;
} // namespace ork::lexer::token

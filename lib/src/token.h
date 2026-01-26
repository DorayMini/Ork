#pragma once
#include <string>
#include <variant>

#include "value.h"

namespace ork::lexer::token {
    enum class KEYWORD {
        FN,
        IF,
        RETURN,
    };
    enum class TYPE {
        INTEGER
    };
    struct LPAR {
        bool operator==(const LPAR&) const = default;
    }; // (
    struct RPAR {
        bool operator==(const RPAR&) const = default;
    }; // )
    struct LBRACE {
        bool operator==(const LBRACE&) const = default;
    }; // {
    struct RBRACE {
        bool operator==(const RBRACE&) const = default;
    }; // }
    struct IDENTIFIER {
        bool operator==(const IDENTIFIER&) const = default;
        std::string_view value;
    };
    struct SEMICOLON {
        bool operator==(const SEMICOLON&) const = default;
    }; // ;
    struct COMMA {
        bool operator==(const COMMA&) const = default;
    }; // ,
    struct INTEGER {
        bool operator==(const INTEGER&) const = default;

        Value value;
    };
    struct STRING {
        bool operator==(const STRING&) const = default;
        std::string_view value;
    };
    struct PLUS {
        bool operator==(const PLUS&) const = default;
    }; // +
    struct MINUS {
        bool operator==(const MINUS&) const = default;
    }; // -
    struct ASTERISK {
        bool operator==(const ASTERISK&) const = default;
    }; // *
    struct SLASH {
        bool operator==(const SLASH&) const = default;
    }; // /
    struct GREATER {
        bool operator==(const GREATER&) const = default;
    }; // >
    struct LESS {
        bool operator==(const LESS&) const = default;
    }; // <
    struct GREATER_EQUAL {
        bool operator==(const GREATER_EQUAL&) const = default;
    }; // >=
    struct LESS_EQUAL {
        bool operator==(const LESS_EQUAL&) const = default;
    }; // <=
    struct EQUAL {
        bool operator==(const EQUAL&) const = default;
    }; // =
    struct OR_OR {
        bool operator==(const OR_OR&) const = default;
    }; // ||
    struct AND_AND {
        bool operator==(const AND_AND&) const = default;
    }; // &&
    struct EQUAL_EQUAL {
        bool operator==(const EQUAL_EQUAL&) const = default;
    }; // ==

    using Lexem = std::variant<
        KEYWORD,
        TYPE,
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
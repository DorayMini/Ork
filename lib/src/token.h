#pragma once
#include <string>
#include <variant>

namespace lexer::token {
    enum class KEYWORD {
        FN,
        RETURN
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
        int32_t value;
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
    struct EQUAL {
        bool operator==(const EQUAL&) const = default;
    };

    using Token = std::variant<
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
        EQUAL
    >;
} // namespace lexer::token
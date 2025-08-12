//
// Created by doray on 8/11/25.
//

#pragma once

#include <memory>
#include <variant>

#include "token.h"
#include "value.h"

namespace expression {
    class Base {
    public:
        virtual ~Base() = default;
        [[nodiscard]] virtual bool equals(const Base& other) const = 0;

    };

    class Constant final : public Base {
    public:
        Value value;

        explicit Constant(const Value &value) : value(value) {}

        ~Constant() override = default;
        bool equals(const Base &other) const override;
    };

    using Op = std::variant<lexer::token::PLUS, lexer::token::MINUS, lexer::token::SLASH, lexer::token::EQUAL, lexer::token::ASTERISK>;
    class Binary final : public Base {
    public:
        Op op;
        std::unique_ptr<Base> lhs;
        std::unique_ptr<Base> rhs;

        Binary(const Op& op, std::unique_ptr<Base> lhs, std::unique_ptr<Base> rhs)
            : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {
        }

        ~Binary() override = default;

        [[nodiscard]] bool equals(const Base& other) const override;
    };

    class Identifier final : public Base {
    public:
        std::string name;
        explicit Identifier(const std::string &name) : name(name) {}
        ~Identifier() override = default;
        [[nodiscard]] bool equals(const Base& other) const override;
    };
} // namespace expression

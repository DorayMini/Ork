//
// Created by doray on 8/11/25.
//

#pragma once

#include <memory>
#include <variant>

#include "token.h"
#include "value.h"

namespace ork::expression {
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

    enum class BinaryOp {
        Minus,
        Plus,
        Asterisk,
        Slash,
        Equal,
    };
    class Binary final : public Base {
    public:
        BinaryOp op;
        std::unique_ptr<Base> lhs;
        std::unique_ptr<Base> rhs;

        Binary(const BinaryOp& op, std::unique_ptr<Base> lhs, std::unique_ptr<Base> rhs)
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

    enum class VarType {
        Int
    };
    class Variable final : public Base {
    public:
        VarType type;
        std::unique_ptr<Identifier> name;
        std::unique_ptr<Base> value = nullptr;

        explicit Variable(VarType type, std::unique_ptr<Identifier> name, std::unique_ptr<Base> value) : type(type), name(std::move(name)), value(std::move(value)) {}
        ~Variable() override = default;
        [[nodiscard]] bool equals(const Base& other) const override;
    };
} // namespace ork::expression

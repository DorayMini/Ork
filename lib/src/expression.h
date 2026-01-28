//
// Created by doray on 8/11/25.
//

#pragma once

#include <string>
#include <memory>
#include <variant>
#include <vector>

#include "token.h"
#include "value.h"

namespace ork::expression {
    class Base {
    public:
        virtual ~Base() = default;

        [[nodiscard]] virtual bool equals(const Base &other) const = 0;
    };

    enum class Type {
        Int32,
        Bool,
        Void
    };

    std::string toString(const Type& t);

    class Constant final : public Base {
    public:
        Type type;
        Value value;

        explicit Constant(Type type, const Value &value) : type(type), value(value) {}

        ~Constant() override = default;

        [[nodiscard]] bool equals(const Base &other) const override;
    };

    enum class BinaryOp {
        Minus,
        Plus,
        Asterisk,
        Slash,
        Equal,
        Greater,
        Less,
        GreaterEqual,
        LessEqual,
        OrOr,
        AndAnd,
        EqualEqual
    };

    class Binary final : public Base {
    public:
        BinaryOp op;
        std::unique_ptr<Base> lhs;
        std::unique_ptr<Base> rhs;

        Binary(const BinaryOp &op, std::unique_ptr<Base> lhs, std::unique_ptr<Base> rhs)
            : op(op), lhs(std::move(lhs)), rhs(std::move(rhs)) {
        }

        ~Binary() override = default;

        [[nodiscard]] bool equals(const Base &other) const override;
    };

    class Identifier final : public Base {
    public:
        std::string name;

        explicit Identifier(const std::string &name) : name(name) {}

        ~Identifier() override = default;

        [[nodiscard]] bool equals(const Base &other) const override;
    };


    class Variable final : public Base {
    public:
        Type type;
        std::unique_ptr<Identifier> name;
        std::unique_ptr<Base> value = nullptr;

        explicit Variable(Type type, std::unique_ptr<Identifier> name, std::unique_ptr<Base> value)
            : type(type), name(std::move(name)), value(std::move(value)) {
        }

        ~Variable() override = default;

        [[nodiscard]] bool equals(const Base &other) const override;
    };

    class FunctionDecl final : public Base {
    public:
        std::unique_ptr<Identifier> name;
        std::vector<std::unique_ptr<Base> > body;
        std::vector<std::unique_ptr<Variable>> arguments;
        Type returnType;

        explicit FunctionDecl(
                std::unique_ptr<Identifier> name,
                std::vector<std::unique_ptr<Base> > body = {},
                std::vector<std::unique_ptr<Variable>> arguments = {},
                Type returnType = Type::Void )
            : name(std::move(name)), body(std::move(body)), arguments(std::move(arguments)), returnType(returnType) {}

        ~FunctionDecl() override = default;

        [[nodiscard]] bool equals(const Base &other) const override;
    };
    class IfStatement final : public Base {
    public:
        std::unique_ptr<Base> condition;
        std::vector<std::unique_ptr<Base>> then;

        explicit IfStatement(std::unique_ptr<Base> condition = {}, std::vector<std::unique_ptr<Base>> then = {}) : condition(std::move(condition)), then(std::move(then)) {}

        ~IfStatement() override = default;

        [[nodiscard]] bool equals(const Base &other) const override;
    };
} // namespace ork::expression

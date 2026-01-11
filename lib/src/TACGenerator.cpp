//
// Created by doray on 8/13/25.
//

#include "TACGenerator.h"

#include <variant>

#include <typeinfo>
#include <functional>
#include <map>

#include "tool.h"

namespace ork {
    bool TACGenerator::Instruction::operator==(const Instruction &other) const {
        if (op != other.op) return false;

        auto varDecl = [](auto &&a, auto &&b) -> bool {
            if (!a && !b) return true;
            if (!a || !b) return false;

            return std::visit(match{
                                  [](const expression::Identifier& id0, const expression::Identifier& id1) -> bool {
                                      return id0.equals(id1);
                                  },
                                  []<typename T0, typename T1>(T0 &&t0, T1 &&t1) -> bool {
                                      using A = std::decay_t<T0>;
                                      using B = std::decay_t<T1>;

                                      if constexpr (std::is_same_v<A, B>) {
                                          return t0 == t1;
                                      }
                                      return false;
                                  }
                              },
                              *a, *b);
        };

        bool arg1Equal = varDecl(arg1, other.arg1);
        bool arg2Equal = varDecl(arg2, other.arg2);
        bool resultEqual = varDecl(result, other.result);

        return arg1Equal && arg2Equal && resultEqual;
    }

    void TACGenerator::Generator::proccess(std::vector<std::unique_ptr<expression::Base>> nodes) {
        for (auto& node: nodes) {
            generate(std::move(node));
        }
        if (!functionNames.contains("main")) {
            throw std::runtime_error("Program must define a main function");
        }

    }

    std::unique_ptr<TACGenerator::Operand> TACGenerator::Generator::generate(std::unique_ptr<expression::Base> node) {
        if (auto i = dynamic_cast<expression::Identifier *>(node.get())) {
            return std::make_unique<Operand>(i->name);
        }
        if (auto c = dynamic_cast<expression::Constant *>(node.get())) {
            if (std::holds_alternative<int32_t>(c->value)) {
                return std::make_unique<Operand>(
                    std::get<int32_t>(c->value));
            }
        }

        if (auto fun = dynamic_cast<expression::FunctionDecl *>(node.get())) {
            instructions.push_back(std::move(Instruction{
                .op = Operation::FUNC_START,
                .result =  std::make_unique<Operand>(fun->name->name),
            }));

            functionNames.emplace(fun->name->name);

            for (auto &body : fun->body) {
                generate(std::move(body));
            }

            instructions.push_back(std::move(Instruction{
                .op = Operation::FUNC_END,
                .result =  std::make_unique<Operand>(fun->name->name),
            }));

            return std::make_unique<Operand>(fun->name->name);
        }
        if (auto b = dynamic_cast<expression::Binary *>(node.get())) {
            auto g_op = getOperation(b->op);
            auto g_arg1 = generate(std::move(b->lhs));
            auto g_arg2 = generate(std::move(b->rhs));

            if (auto fold = foldVariable(g_op, *g_arg1, *g_arg2)) {
                 return std::move(*fold);
            }

            std::string result = fmt::format("f{}", counter++);
            instructions.push_back(std::move(Instruction{
                .op = g_op,
                .arg1 = std::move(g_arg1),
                .arg2 = std::move(g_arg2),
                .result = std::make_unique<Operand>(result)
            }));
            return std::make_unique<Operand>(result);
        }
        if (auto var = dynamic_cast<expression::Variable *>(node.get())) {
            instructions.push_back(std::move(Instruction{
                .op = Operation::MOV,
                .arg1 = generate(std::move(var->value)),
                .result = std::make_unique<Operand>(var->name->name)
            }));

            return std::make_unique<Operand>(var->name->name);
        }

        throw std::runtime_error(fmt::format("TACGenerator -> Unknown node type {}", typeid(*node).name()));
    }

    std::vector<TACGenerator::Instruction> TACGenerator::Generator::takeInstructions() {
        return std::move(instructions);
    }

    TACGenerator::Operation TACGenerator::Generator::getOperation(const expression::BinaryOp &node) {
        switch (node) {
            case expression::BinaryOp::Plus:
                return Operation::ADD;
            case expression::BinaryOp::Minus:
                return Operation::SUB;
            case expression::BinaryOp::Asterisk:
                return Operation::MULT;
            case expression::BinaryOp::Slash:
                return Operation::DIV;
            default:
                throw std::runtime_error("TACgenerator -> Unknown operation type");
        }
    }

    std::optional<std::unique_ptr<TACGenerator::Operand>> TACGenerator::Generator::foldVariable(const Operation& op, const Operand& val1, const Operand& val2) {
        if (std::holds_alternative<Value>(val1)) {
            if (std::holds_alternative<Value>(val2)) {
                return std::make_unique<Operand>(std::visit([&](auto &&a, auto &&b) -> Value {
                    using T1 = std::decay_t<decltype(a)>;
                    using T2 = std::decay_t<decltype(b)>;
                    std::map<Operation, std::function<Value(T1, T2)> > operation{
                        {Operation::ADD, std::plus<T1>()},
                        {Operation::SUB, std::minus<T1>()},
                        {Operation::MULT, std::multiplies<T1>()},
                        {Operation::DIV, std::divides<T1>()}
                    };

                    if (auto fun = operation.find(op); fun != operation.end()) {
                        return fun->second(a, b);
                    }

                    return Value{};
                }, std::get<Value>(val1), std::get<Value>(val2)));
            }
        }

        return std::nullopt;
    }
} // nanamespace ork

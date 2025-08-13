//
// Created by doray on 8/11/25.
//

#include "expression.h"

namespace expression {
    bool Constant::equals(const Base& other) const {
        auto otherConstant = dynamic_cast<const Constant*>(&other);
        if (!otherConstant) {
            return false;
        }
        return value == otherConstant->value;
    }

    bool Binary::equals(const Base& other) const {
        auto otherB = dynamic_cast<const Binary*>(&other);
        if (!otherB) {
            return false;
        }

        if (op != otherB->op) {
            return false;
        }

        bool lhsEqual = false;

        if (auto lhsBase = dynamic_cast<const Base*>(lhs.get())) {
            if (auto otherBase = dynamic_cast<const Base*>(otherB->lhs.get())) {
                lhsEqual = lhsBase->equals(*otherBase);
            }
        } else
            return false;

        bool rhsEqual = false;

        if (auto rhsBase = dynamic_cast<const Base*>(rhs.get())) {
            if (auto otherBase = dynamic_cast<const Base*>(otherB->rhs.get())) {
                rhsEqual = rhsBase->equals(*otherBase);
            }
        } else
            return false;

        return lhsEqual && rhsEqual;
    }

    bool Identifier::equals(const Base &other) const {
        auto otherIdentifier = dynamic_cast<const Identifier*>(&other);
        if (!otherIdentifier) {
            return false;
        }
        return (name == otherIdentifier->name);
    }

    bool Variable::equals(const Base &other) const {
        auto otherVariable = dynamic_cast<const Variable*>(&other);
        if (!otherVariable) {
            return false;
        }
        if (otherVariable->type != type) {
            return false;
        }

        bool lhsEqual = false;
        bool rhsEqual = false;
        if (name->equals(*otherVariable->name)) {
            lhsEqual = true;
            if (value->equals(*otherVariable->value)) {
                rhsEqual = true;
            }
        }
        return lhsEqual && rhsEqual;
    }
} // namespace expression
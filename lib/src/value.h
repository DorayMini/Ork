//
// Created by doray on 8/11/25.
//

#pragma once
#include <variant>

namespace ork {
    using Value = std::variant<
        int, bool
    >;
} // namespace ork

#pragma once

#include <any>
#include <string_view>

class graphl {
public:
    std::any execute(std::string_view str);
};

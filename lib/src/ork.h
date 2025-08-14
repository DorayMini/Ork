#pragma once

#include <any>
#include <string_view>

class ork {
public:
    std::any execute(std::string_view str);
};

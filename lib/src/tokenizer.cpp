//
// Created by doray on 8/11/25.
//
#include "tokenizer.h"

#include <charconv>
#include <fmt/format.h>
#include <locale>
#include <map>

namespace ork {
    std::vector<lexer::token::Lexem> lexer::proccess(std::string_view input) {
        std::vector<token::Lexem> tokens;
        int line = 1;

        for (auto iter = input.begin(); iter != input.end(); ++iter) {
            auto remainingString = std::ranges::subrange(iter, input.end());
            switch (*iter) {
                default:
                    if (std::isalpha(*iter)) {
                        auto blank = std::ranges::find_if(remainingString, [](char c) {
                            return !std::isalnum(c) && c != '_';
                        });

                        std::string_view valueString(iter, blank);
                        iter = std::prev(blank);

                        static std::map<std::string_view, token::KEYWORD> keywords = {
                            {"fn", token::KEYWORD::FN},
                            {"return", token::KEYWORD::RETURN},
                        };

                        if (auto keyword = keywords.find(valueString); keyword != keywords.end()) {
                            tokens.emplace_back(keyword->second);
                            break;
                        }

                        static std::map<std::string_view, token::TYPE> types = {
                            {"int", token::TYPE::INTEGER}
                        };

                        if (auto type = types.find(valueString); type != types.end()) {
                            tokens.emplace_back(type->second);
                            break;
                        }

                        tokens.emplace_back(token::IDENTIFIER{ .value = valueString });
                        break;
                    }
                    if (isdigit(*iter)) {
                        auto blank = std::ranges::find_if(remainingString, [](char c) {
                            return !std::isdigit(c);
                        });

                        std::string_view digitString(iter, blank);
                        int number = 0;
                        std::from_chars(digitString.data(), digitString.data() + digitString.size(), number);
                        tokens.emplace_back(token::INTEGER{ .value = number });

                        iter = std::prev(blank);
                        break;
                    }

                    throw std::runtime_error(fmt::format("unexpected character: {}, at line {}", *iter, line));
                case ' ':
                case '\r':
                case '\t':
                    break;
                case '\n':
                    line++;
                    break;
                case ';': tokens.emplace_back(token::SEMICOLON{}); break;
                case ',': tokens.emplace_back(token::COMMA{});     break;
                case '(': tokens.emplace_back(token::LPAR{});      break;
                case ')': tokens.emplace_back(token::RPAR{});      break;
                case '{': tokens.emplace_back(token::LBRACE{});    break;
                case '}': tokens.emplace_back(token::RBRACE{});    break;
                case '+': tokens.emplace_back(token::PLUS{});      break;
                case '-': tokens.emplace_back(token::MINUS{});     break;
                case '*': tokens.emplace_back(token::ASTERISK{});  break;
                case '/': tokens.emplace_back(token::SLASH{});     break;
                case '=': tokens.emplace_back(token::EQUAL{});     break;
                case '"': {
                    auto blank = std::ranges::find(remainingString, '"');

                    if (blank == remainingString.end()) {
                        throw std::runtime_error(fmt::format("unterminated string starting with '{}' at line {}", *iter, line));
                    }
                    std::string_view textString(iter, blank);
                    tokens.emplace_back(token::STRING { .value = textString });
                    break;
                }

            }
            if (iter == input.end()) {
                break;
            }
        }

        return tokens;
    }
} // namespace ork

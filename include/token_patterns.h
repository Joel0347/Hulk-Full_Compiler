#pragma once
#include "token.h"
#include <unordered_map>
#include <string>
#include <regex>

namespace TokenPatterns {
    const std::unordered_map<TokenType, std::string> patterns = {
        {TOK_FUNCTION, "function"},
        {TOK_LET, "let"},
        {TOK_IN, "in"},
        {TOK_IF, "if"},
        {TOK_ELIF, "elif"},
        {TOK_ELSE, "else"},
        {TOK_TRUE, "true"},
        {TOK_FALSE, "false"},
        {TOK_PI, "PI"},
        {TOK_E, "E"},
        {TOK_IDENTIFIER, "[a-zA-Z_][a-zA-Z0-9_]*"},
        {TOK_NUMBER, "([0-9]+(\\.[0-9]*)?|\\.[0-9]+)([eE][+-]?[0-9]+)?"},
        {TOK_STRING, "\"(\\\\.|[^\"])*\""},
        {TOK_PLUS, "\\+"},
        {TOK_MINUS, "-"},
        {TOK_MULTIPLY, "\\*"},
        {TOK_DIVIDE, "/"},
        {TOK_POWER, "\\^"},
        {TOK_MODULO, "%"},
        {TOK_CONCAT, "@"},
        {TOK_DOUBLE_CONCAT, "@@"},
        {TOK_ASSIGN, "="},
        {TOK_REASSIGN, ":="},
        {TOK_EQUAL, "=="},
        {TOK_NOT_EQUAL, "!="},
        {TOK_LESS, "<"},
        {TOK_GREATER, ">"},
        {TOK_LESS_EQUAL, "<="},
        {TOK_GREATER_EQUAL, ">="},
        {TOK_AND, "&"},
        {TOK_OR, "\\|"},
        {TOK_NOT, "!"},
        {TOK_LPAREN, "\\("},
        {TOK_RPAREN, "\\)"},
        {TOK_LBRACE, "\\{"},
        {TOK_RBRACE, "\\}"},
        {TOK_SEMICOLON, ";"},
        {TOK_COMMA, ","},
        {TOK_ARROW, "=>"}
    };

    static std::unordered_map<TokenType, std::regex> compiledPatterns;
    
    inline void initializePatterns() {
        if (compiledPatterns.empty()) {
            for (const auto& pattern : patterns) {
                compiledPatterns[pattern.first] = std::regex(pattern.second);
            }
        }
    }
}

#pragma once
#include "Token.h"
#include <string>

class Lexer {
private:
    std::string input;
    size_t position;
    
    char peek() const;
    void advance();
    void skipWhitespace();
    Token getNumber();

public:
    Lexer(const std::string& source) : input(source), position(0) {}
    Token getNextToken();
};

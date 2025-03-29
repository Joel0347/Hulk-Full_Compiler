#pragma once
#include "Lexer.h"

class Parser {
private:
    Lexer lexer;
    Token currentToken;
    
    void eat(TokenType type);
    int factor();
    int term();
    int expr();

public:
    Parser(const std::string& input) : lexer(input), currentToken(lexer.getNextToken()) {
    }
    
    void parse();
};

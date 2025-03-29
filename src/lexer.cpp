#include "Lexer.h"
#include <stdexcept>
#include <cctype>

char Lexer::peek() const {
    if (position >= input.length()) return '\0';
    return input[position];
}

void Lexer::advance() {
    position++;
}

void Lexer::skipWhitespace() {
    while (peek() == ' ' || peek() == '\t' || peek() == '\n' || peek() == '\r') {
        advance();
    }
}

Token Lexer::getNumber() {
    std::string number;
    while (isdigit(peek())) {
        number += peek();
        advance();
    }
    return Token(TokenType::NUMBER, number);
}

Token Lexer::getNextToken() {
    skipWhitespace();
    
    if (peek() == '\0') return Token(TokenType::END);
    
    if (isdigit(peek())) {
        return getNumber();
    }
    
    switch (peek()) {
        case '+':
            advance();
            return Token(TokenType::PLUS);
        case '-':
            advance();
            return Token(TokenType::MINUS);
        case '*':
            advance();
            return Token(TokenType::MULTIPLY);
        case '/':
            advance();
            return Token(TokenType::DIVIDE);
        case '(':
            advance();
            return Token(TokenType::LPAREN);
        case ')':
            advance();
            return Token(TokenType::RPAREN);
        case '\n':
            advance();
            return Token(TokenType::EOL);
        default:
            throw std::runtime_error("Carácter no reconocido: " + std::string(1, peek()));
    }
}

#include "Parser.h"
#include <stdexcept>
#include <string>

void Parser::eat(TokenType type) {
    if (currentToken.type == type) {
        currentToken = lexer.getNextToken();
    } else {
        throw std::runtime_error("Error de sintaxis");
    }
}

int Parser::factor() {
    Token token = currentToken;
    if (token.type == TokenType::NUMBER) {
        eat(TokenType::NUMBER);
        return std::stoi(token.value);
    } else if (token.type == TokenType::LPAREN) {
        eat(TokenType::LPAREN);
        int result = expr();
        eat(TokenType::RPAREN);
        return result;
    }
    throw std::runtime_error("Factor inválido");
}

int Parser::term() {
    factor();
    while (currentToken.type == TokenType::MULTIPLY || 
           currentToken.type == TokenType::DIVIDE) {
        Token token = currentToken;
        if (token.type == TokenType::MULTIPLY) {
            eat(TokenType::MULTIPLY);
            factor();
        } else if (token.type == TokenType::DIVIDE) {
            eat(TokenType::DIVIDE);
            factor();
        }
    }
    return 0; // Solo parsing, no evaluación
}

int Parser::expr() {
    term();
    while (currentToken.type == TokenType::PLUS || 
           currentToken.type == TokenType::MINUS) {
        Token token = currentToken;
        if (token.type == TokenType::PLUS) {
            eat(TokenType::PLUS);
            term();
        } else if (token.type == TokenType::MINUS) {
            eat(TokenType::MINUS);
            term();
        }
    }
    return 0; // Solo parsing, no evaluación
}

void Parser::parse() {
    expr();
    if (currentToken.type != TokenType::END && 
        currentToken.type != TokenType::EOL) {
        throw std::runtime_error("Error de sintaxis: tokens adicionales al final");
    }
}

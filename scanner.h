//
// Created by ryanh on 5/9/2026.
//
#ifndef CLOX_SCANNER_H
#define CLOX_SCANNER_H

#include <string>

enum TokenType {
    // Single-character tokens.
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_DOT, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,

    // One or two character tokens.
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,

    // Literals.
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,

    // Keywords.
    TOKEN_AND, TOKEN_CLASS, TOKEN_ELSE, TOKEN_FALSE,
    TOKEN_FOR, TOKEN_FUN, TOKEN_IF, TOKEN_NIL, TOKEN_OR,
    TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,

    TOKEN_ERROR,
    TOKEN_EOF
};

struct Token {
    TokenType type;
    const char* start;
    int length;
    int line;
};

class Scanner {
public:
    explicit Scanner(const std::string& source);

    Token scanToken();

private:
    const char* start;
    const char* current;
    int line;

    bool isAtEnd() const;

    bool isAlpha(char c) const;
    bool isDigit(char c) const;

    char advance();
    bool match(char expected);
    char peek() const;
    char peekNext() const;

    void skipWhitespace();

    Token makeToken(TokenType type) const;
    Token errorToken(const char* message) const;

    Token string();
    Token number();
    Token identifier();

    TokenType identifierType() const;
    TokenType checkKeyword(int startIndex, int length, const char* rest, TokenType type) const;
};

#endif
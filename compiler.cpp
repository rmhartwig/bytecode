//
// Created by ryanh on 5/9/2026.
//
#include "compiler.h"

#include "common.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

#include <cstdlib>
#include <iostream>

struct Parser {
    Token current;
    Token previous;
    bool hadError = false;
    bool panicMode = false;
};

enum Precedence {
    PREC_NONE,
    PREC_ASSIGNMENT,  // =
    PREC_OR,          // or
    PREC_AND,         // and
    PREC_EQUALITY,    // == !=
    PREC_COMPARISON,  // < > <= >=
    PREC_TERM,        // + -
    PREC_FACTOR,      // * /
    PREC_UNARY,       // ! -
    PREC_CALL,        // . ()
    PREC_PRIMARY
};

using ParseFn = void (*)();

struct ParseRule {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
};

static Parser parser;
static Scanner* currentScanner = nullptr;
static Chunk* compilingChunk = nullptr;

static Chunk* currentChunk() {
    return compilingChunk;
}

static void expression();
static void parsePrecedence(Precedence precedence);
static ParseRule* getRule(TokenType type);

static void errorAt(Token* token, const char* message) {
    if (parser.panicMode) return;

    parser.panicMode = true;

    std::cerr << "[line " << token->line << "] Error";

    if (token->type == TOKEN_EOF) {
        std::cerr << " at end";
    } else if (token->type == TOKEN_ERROR) {
        // Nothing.
    } else {
        std::cerr << " at '"
                  << std::string(token->start, token->length)
                  << "'";
    }

    std::cerr << ": " << message << "\n";

    parser.hadError = true;
}

static void errorAtCurrent(const char* message) {
    errorAt(&parser.current, message);
}

static void error(const char* message) {
    errorAt(&parser.previous, message);
}

static void advance() {
    parser.previous = parser.current;

    for (;;) {
        parser.current = currentScanner->scanToken();

        if (parser.current.type != TOKEN_ERROR) {
            break;
        }

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char* message) {
    if (parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}

static void emitByte(uint8_t byte) {
    currentChunk()->write(byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn() {
    emitByte(OP_RETURN);
}

static uint8_t makeConstant(Value value) {
    int constant = currentChunk()->addConstant(value);

    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk.");
        return 0;
    }

    return static_cast<uint8_t>(constant);
}

static void emitConstant(Value value) {
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void endCompiler() {
    emitReturn();

#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError) {
        disassembleChunk(*currentChunk(), "code");
    }
#endif
}

static void number() {
    double value = std::strtod(parser.previous.start, nullptr);
    emitConstant(NUMBER_VAL(value));
}

static void grouping() {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void unary() {
    TokenType operatorType = parser.previous.type;

    parsePrecedence(PREC_UNARY);

    switch (operatorType) {
        case TOKEN_BANG:
            emitByte(OP_NOT);
            break;

        case TOKEN_MINUS:
            emitByte(OP_NEGATE);
            break;

        default:
            return; // Unreachable.
    }
}

static void binary() {
    TokenType operatorType = parser.previous.type;

    ParseRule* rule = getRule(operatorType);
    parsePrecedence(static_cast<Precedence>(rule->precedence + 1));

    switch (operatorType) {
        case TOKEN_BANG_EQUAL:
            emitBytes(OP_EQUAL, OP_NOT);
            break;

        case TOKEN_EQUAL_EQUAL:
            emitByte(OP_EQUAL);
            break;

        case TOKEN_GREATER:
            emitByte(OP_GREATER);
            break;

        case TOKEN_GREATER_EQUAL:
            emitBytes(OP_LESS, OP_NOT);
            break;

        case TOKEN_LESS:
            emitByte(OP_LESS);
            break;

        case TOKEN_LESS_EQUAL:
            emitBytes(OP_GREATER, OP_NOT);
            break;

        case TOKEN_PLUS:
            emitByte(OP_ADD);
            break;

        case TOKEN_MINUS:
            emitByte(OP_SUBTRACT);
            break;

        case TOKEN_STAR:
            emitByte(OP_MULTIPLY);
            break;

        case TOKEN_SLASH:
            emitByte(OP_DIVIDE);
            break;

        default:
            return; // Unreachable.
    }
}

static void literal() {
    switch (parser.previous.type) {
        case TOKEN_FALSE:
            emitByte(OP_FALSE);
            break;

        case TOKEN_NIL:
            emitByte(OP_NIL);
            break;

        case TOKEN_TRUE:
            emitByte(OP_TRUE);
            break;

        default:
            return; // Unreachable.
    }
}
static ParseRule rules[] = {
    {grouping, nullptr, PREC_NONE},       // TOKEN_LEFT_PAREN
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_RIGHT_PAREN
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_LEFT_BRACE
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_RIGHT_BRACE
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_COMMA
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_DOT
    {unary,    binary,  PREC_TERM},       // TOKEN_MINUS
    {nullptr,  binary,  PREC_TERM},       // TOKEN_PLUS
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_SEMICOLON
    {nullptr,  binary,  PREC_FACTOR},     // TOKEN_SLASH
    {nullptr,  binary,  PREC_FACTOR},     // TOKEN_STAR

    {unary,    nullptr, PREC_NONE},       // TOKEN_BANG
    {nullptr,  binary,  PREC_EQUALITY},   // TOKEN_BANG_EQUAL
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_EQUAL
    {nullptr,  binary,  PREC_EQUALITY},   // TOKEN_EQUAL_EQUAL
    {nullptr,  binary,  PREC_COMPARISON}, // TOKEN_GREATER
    {nullptr,  binary,  PREC_COMPARISON}, // TOKEN_GREATER_EQUAL
    {nullptr,  binary,  PREC_COMPARISON}, // TOKEN_LESS
    {nullptr,  binary,  PREC_COMPARISON}, // TOKEN_LESS_EQUAL

    {nullptr,  nullptr, PREC_NONE},       // TOKEN_IDENTIFIER
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_STRING
    {number,   nullptr, PREC_NONE},       // TOKEN_NUMBER

    {nullptr,  nullptr, PREC_NONE},       // TOKEN_AND
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_CLASS
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_ELSE
    {literal,  nullptr, PREC_NONE},       // TOKEN_FALSE
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_FOR
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_FUN
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_IF
    {literal,  nullptr, PREC_NONE},       // TOKEN_NIL
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_OR
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_PRINT
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_RETURN
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_SUPER
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_THIS
    {literal,  nullptr, PREC_NONE},       // TOKEN_TRUE
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_VAR
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_WHILE

    {nullptr,  nullptr, PREC_NONE},       // TOKEN_ERROR
    {nullptr,  nullptr, PREC_NONE},       // TOKEN_EOF
};

static ParseRule* getRule(TokenType type) {
    return &rules[type];
}

static void parsePrecedence(Precedence precedence) {
    advance();

    ParseFn prefixRule = getRule(parser.previous.type)->prefix;

    if (prefixRule == nullptr) {
        error("Expect expression.");
        return;
    }

    prefixRule();

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();

        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule();
    }
}

static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

bool compile(const std::string& source, Chunk& chunk) {
    Scanner scanner(source);

    currentScanner = &scanner;
    compilingChunk = &chunk;

    parser.hadError = false;
    parser.panicMode = false;

    advance();

    expression();

    consume(TOKEN_EOF, "Expect end of expression.");

    endCompiler();

    return !parser.hadError;
}
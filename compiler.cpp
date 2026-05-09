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
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY
};

using ParseFn = void (*)(bool canAssign);

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
static void statement();
static void declaration();

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

static bool check(TokenType type) {
    return parser.current.type == type;
}

static bool match(TokenType type) {
    if (!check(type)) return false;

    advance();
    return true;
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

static uint8_t identifierConstant(Token* name) {
    return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static uint8_t parseVariable(const char* errorMessage) {
    consume(TOKEN_IDENTIFIER, errorMessage);
    return identifierConstant(&parser.previous);
}

static void defineVariable(uint8_t global) {
    emitBytes(OP_DEFINE_GLOBAL, global);
}

static void number(bool canAssign) {
    double value = std::strtod(parser.previous.start, nullptr);
    emitConstant(NUMBER_VAL(value));
}

static void string(bool canAssign) {
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1,
                                    parser.previous.length - 2)));
}

static void literal(bool canAssign) {
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
            return;
    }
}

static void grouping(bool canAssign) {
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void unary(bool canAssign) {
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
            return;
    }
}

static void binary(bool canAssign) {
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
            return;
    }
}

static void namedVariable(Token name, bool canAssign) {
    uint8_t arg = identifierConstant(&name);

    if (canAssign && match(TOKEN_EQUAL)) {
        expression();
        emitBytes(OP_SET_GLOBAL, arg);
    } else {
        emitBytes(OP_GET_GLOBAL, arg);
    }
}

static void variable(bool canAssign) {
    namedVariable(parser.previous, canAssign);
}

static ParseRule rules[] = {
    {grouping, nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {unary,    binary,  PREC_TERM},
    {nullptr,  binary,  PREC_TERM},
    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  binary,  PREC_FACTOR},
    {nullptr,  binary,  PREC_FACTOR},

    {unary,    nullptr, PREC_NONE},
    {nullptr,  binary,  PREC_EQUALITY},
    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  binary,  PREC_EQUALITY},
    {nullptr,  binary,  PREC_COMPARISON},
    {nullptr,  binary,  PREC_COMPARISON},
    {nullptr,  binary,  PREC_COMPARISON},
    {nullptr,  binary,  PREC_COMPARISON},

    {variable, nullptr, PREC_NONE},
    {string,   nullptr, PREC_NONE},
    {number,   nullptr, PREC_NONE},

    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {literal,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {literal,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {literal,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},

    {nullptr,  nullptr, PREC_NONE},
    {nullptr,  nullptr, PREC_NONE},
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

    bool canAssign = precedence <= PREC_ASSIGNMENT;

    prefixRule(canAssign);

    while (precedence <= getRule(parser.current.type)->precedence) {
        advance();

        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }

    if (canAssign && match(TOKEN_EQUAL)) {
        error("Invalid assignment target.");
    }
}

static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

static void printStatement() {
    expression();

    consume(TOKEN_SEMICOLON, "Expect ';' after value.");

    emitByte(OP_PRINT);
}

static void expressionStatement() {
    expression();

    consume(TOKEN_SEMICOLON, "Expect ';' after expression.");

    emitByte(OP_POP);
}

static void synchronize() {
    parser.panicMode = false;

    while (parser.current.type != TOKEN_EOF) {
        if (parser.previous.type == TOKEN_SEMICOLON) return;

        switch (parser.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;

            default:
                break;
        }

        advance();
    }
}

static void statement() {
    if (match(TOKEN_PRINT)) {
        printStatement();
    } else {
        expressionStatement();
    }
}

static void varDeclaration() {
    uint8_t global = parseVariable("Expect variable name.");

    if (match(TOKEN_EQUAL)) {
        expression();
    } else {
        emitByte(OP_NIL);
    }

    consume(TOKEN_SEMICOLON, "Expect ';' after variable declaration.");

    defineVariable(global);
}

static void declaration() {
    if (match(TOKEN_VAR)) {
        varDeclaration();
    } else {
        statement();
    }

    if (parser.panicMode) {
        synchronize();
    }
}

bool compile(const std::string& source, Chunk& chunk) {
    Scanner scanner(source);

    currentScanner = &scanner;
    compilingChunk = &chunk;

    parser.hadError = false;
    parser.panicMode = false;

    advance();

    while (!match(TOKEN_EOF)) {
        declaration();
    }

    endCompiler();

    return !parser.hadError;
}
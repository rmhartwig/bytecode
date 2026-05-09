//
// Created by ryanh on 5/9/2026.
//
#ifndef CLOX_CHUNK_H
#define CLOX_CHUNK_H

#include "common.h"
#include "value.h"

#include <vector>

enum OpCode {
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,

    OP_POP,

    OP_GET_LOCAL,
    OP_SET_LOCAL,

    OP_GET_GLOBAL,
    OP_DEFINE_GLOBAL,
    OP_SET_GLOBAL,

    OP_EQUAL,
    OP_GREATER,
    OP_LESS,

    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,

    OP_NOT,
    OP_NEGATE,

    OP_PRINT,

    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,

    OP_RETURN
};

class Chunk {
public:
    void write(uint8_t byte, int line);
    int addConstant(Value value);
    void setCode(int index, uint8_t byte);

    int count() const;
    uint8_t getCode(int index) const;
    int getLine(int index) const;
    Value getConstant(int index) const;

private:
    std::vector<uint8_t> code;
    std::vector<int> lines;
    ValueArray constants;
};

#endif
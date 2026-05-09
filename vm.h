//
// Created by ryanh on 5/9/2026.
//
#ifndef CLOX_VM_H
#define CLOX_VM_H

#include "chunk.h"
#include "value.h"

#include <vector>

enum InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

class VM {
public:
    InterpretResult interpret(const Chunk& chunk);

private:
    const Chunk* chunk = nullptr;
    int ip = 0;

    std::vector<Value> stack;

    InterpretResult run();

    uint8_t readByte();
    Value readConstant();

    void push(Value value);
    Value pop();
};

#endif
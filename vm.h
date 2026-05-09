//
// Created by ryanh on 5/9/2026.
//
#ifndef CLOX_VM_H
#define CLOX_VM_H

#include "chunk.h"
#include "table.h"
#include "value.h"

#include <string>
#include <vector>

enum InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

class VM {
public:
    ~VM();

    InterpretResult interpret(const std::string& source);

private:
    const Chunk* chunk = nullptr;
    int ip = 0;

    std::vector<Value> stack;

    Table globals;

    InterpretResult run();

    uint8_t readByte();
    Value readConstant();
    ObjString* readString();

    void push(Value value);
    Value pop();
    Value peek(int distance) const;

    bool isFalsey(Value value) const;
    void concatenate();
    void runtimeError(const char* format, ...);
};

#endif
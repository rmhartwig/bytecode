//
// Created by ryanh on 5/9/2026.
//
#ifndef CLOX_VM_H
#define CLOX_VM_H

#include "chunk.h"
#include "object.h"
#include "table.h"
#include "value.h"

#include <string>
#include <vector>

#define FRAMES_MAX 64

enum InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

struct CallFrame {
    ObjFunction* function = nullptr;
    int ip = 0;
    int slotStart = 0;
};

class VM {
public:
    VM();
    ~VM();

    InterpretResult interpret(const std::string& source);

private:
    std::vector<Value> stack;
    std::vector<CallFrame> frames;

    Table globals;

    InterpretResult run();

    bool call(ObjFunction* function, int argCount);
    bool callValue(Value callee, int argCount);

    uint8_t readByte();
    uint16_t readShort();
    Value readConstant();
    ObjString* readString();

    void push(Value value);
    Value pop();
    Value peek(int distance) const;

    CallFrame& currentFrame();
    Value getStackSlot(int slot) const;
    void setStackSlot(int slot, Value value);

    bool isFalsey(Value value) const;
    void concatenate();
    void defineNative(const char* name, NativeFn function);
    void resetStack();
    void runtimeError(const char* format, ...);
};

#endif
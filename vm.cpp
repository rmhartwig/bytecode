//
// Created by ryanh on 5/9/2026.
//
#include "vm.h"

#include "common.h"
#include "debug.h"

#include <iostream>

InterpretResult VM::interpret(const Chunk& newChunk) {
    chunk = &newChunk;
    ip = 0;
    stack.clear();

    return run();
}

InterpretResult VM::run() {
    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        std::cout << "          ";

        for (Value value : stack) {
            std::cout << "[ " << value << " ]";
        }

        std::cout << "\n";

        disassembleInstruction(*chunk, ip);
#endif

        uint8_t instruction = readByte();

        switch (instruction) {
            case OP_CONSTANT: {
                Value constant = readConstant();
                push(constant);
                break;
            }

            case OP_ADD: {
                Value b = pop();
                Value a = pop();
                push(a + b);
                break;
            }

            case OP_SUBTRACT: {
                Value b = pop();
                Value a = pop();
                push(a - b);
                break;
            }

            case OP_MULTIPLY: {
                Value b = pop();
                Value a = pop();
                push(a * b);
                break;
            }

            case OP_DIVIDE: {
                Value b = pop();
                Value a = pop();
                push(a / b);
                break;
            }

            case OP_NEGATE: {
                push(-pop());
                break;
            }

            case OP_RETURN: {
                std::cout << pop() << "\n";
                return INTERPRET_OK;
            }

            default:
                std::cout << "Unknown opcode " << static_cast<int>(instruction) << "\n";
                return INTERPRET_RUNTIME_ERROR;
        }
    }
}

uint8_t VM::readByte() {
    return chunk->getCode(ip++);
}

Value VM::readConstant() {
    return chunk->getConstant(readByte());
}

void VM::push(Value value) {
    stack.push_back(value);
}

Value VM::pop() {
    Value value = stack.back();
    stack.pop_back();
    return value;
}
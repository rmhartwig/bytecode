//
// Created by ryanh on 5/9/2026.
//
#include "vm.h"

#include "common.h"
#include "compiler.h"
#include "debug.h"
#include "object.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <iostream>

VM::~VM() {
    globals.clear();
    freeObjects();
}

InterpretResult VM::interpret(const std::string& source) {
    Chunk newChunk;

    if (!compile(source, newChunk)) {
        return INTERPRET_COMPILE_ERROR;
    }

    chunk = &newChunk;
    ip = 0;
    stack.clear();

    InterpretResult result = run();

    return result;
}

InterpretResult VM::run() {
#define READ_BYTE() readByte()
#define READ_CONSTANT() readConstant()
#define READ_STRING() readString()

#define BINARY_OP(valueWrapper, op)                         \
    do {                                                    \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) {   \
            runtimeError("Operands must be numbers.");      \
            return INTERPRET_RUNTIME_ERROR;                 \
        }                                                   \
        double b = AS_NUMBER(pop());                        \
        double a = AS_NUMBER(pop());                        \
        push(valueWrapper(a op b));                         \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        std::cout << "          ";

        for (Value value : stack) {
            std::cout << "[ ";
            printValue(value);
            std::cout << " ]";
        }

        std::cout << "\n";

        disassembleInstruction(*chunk, ip);
#endif

        uint8_t instruction = READ_BYTE();

        switch (instruction) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }

            case OP_NIL:
                push(NIL_VAL());
                break;

            case OP_TRUE:
                push(BOOL_VAL(true));
                break;

            case OP_FALSE:
                push(BOOL_VAL(false));
                break;

            case OP_POP:
                pop();
                break;

            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();

                Value value;
                if (!globals.get(name, &value)) {
                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                push(value);
                break;
            }

            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();

                globals.set(name, peek(0));
                pop();

                break;
            }

            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();

                if (globals.set(name, peek(0))) {
                    globals.remove(name);

                    runtimeError("Undefined variable '%s'.", name->chars);
                    return INTERPRET_RUNTIME_ERROR;
                }

                break;
            }

            case OP_EQUAL: {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }

            case OP_GREATER:
                BINARY_OP(BOOL_VAL, >);
                break;

            case OP_LESS:
                BINARY_OP(BOOL_VAL, <);
                break;

            case OP_ADD: {
                if (IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                    concatenate();
                } else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                } else {
                    runtimeError("Operands must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                break;
            }

            case OP_SUBTRACT:
                BINARY_OP(NUMBER_VAL, -);
                break;

            case OP_MULTIPLY:
                BINARY_OP(NUMBER_VAL, *);
                break;

            case OP_DIVIDE:
                BINARY_OP(NUMBER_VAL, /);
                break;

            case OP_NOT:
                push(BOOL_VAL(isFalsey(pop())));
                break;

            case OP_NEGATE:
                if (!IS_NUMBER(peek(0))) {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;

            case OP_PRINT:
                printValue(pop());
                std::cout << "\n";
                break;

            case OP_RETURN:
                return INTERPRET_OK;

            default:
                std::cout << "Unknown opcode "
                          << static_cast<int>(instruction)
                          << "\n";
                return INTERPRET_RUNTIME_ERROR;
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

uint8_t VM::readByte() {
    return chunk->getCode(ip++);
}

Value VM::readConstant() {
    return chunk->getConstant(readByte());
}

ObjString* VM::readString() {
    return AS_STRING(readConstant());
}

void VM::push(Value value) {
    stack.push_back(value);
}

Value VM::pop() {
    Value value = stack.back();
    stack.pop_back();
    return value;
}

Value VM::peek(int distance) const {
    return stack[stack.size() - 1 - distance];
}

bool VM::isFalsey(Value value) const {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

void VM::concatenate() {
    ObjString* b = AS_STRING(pop());
    ObjString* a = AS_STRING(pop());

    int length = a->length + b->length;
    char* chars = new char[length + 1];

    std::memcpy(chars, a->chars, a->length);
    std::memcpy(chars + a->length, b->chars, b->length);

    chars[length] = '\0';

    ObjString* result = takeString(chars, length);
    push(OBJ_VAL(result));
}

void VM::runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);

    std::fputs("\n", stderr);

    int instruction = ip - 1;
    int line = chunk->getLine(instruction);

    std::fprintf(stderr, "[line %d] in script\n", line);

    stack.clear();
}
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
#include <ctime>
#include <iostream>

static Value clockNative(int argCount, Value* args) {
    return NUMBER_VAL(static_cast<double>(std::clock()) / CLOCKS_PER_SEC);
}

VM::VM() {
    defineNative("clock", clockNative);
}

VM::~VM() {
    globals.clear();
    freeObjects();
}

void VM::resetStack() {
    stack.clear();
    frames.clear();
}

InterpretResult VM::interpret(const std::string& source) {
    resetStack();

    ObjFunction* function = compile(source);

    if (function == nullptr) {
        return INTERPRET_COMPILE_ERROR;
    }

    push(OBJ_VAL(function));

    call(function, 0);

    return run();
}

InterpretResult VM::run() {
#define READ_BYTE() readByte()
#define READ_SHORT() readShort()
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

        CallFrame& traceFrame = currentFrame();
        disassembleInstruction(traceFrame.function->chunk, traceFrame.ip);
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

            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(getStackSlot(slot));
                break;
            }

            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                setStackSlot(slot, peek(0));
                break;
            }

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

            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                currentFrame().ip += offset;
                break;
            }

            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();

                if (isFalsey(peek(0))) {
                    currentFrame().ip += offset;
                }

                break;
            }

            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                currentFrame().ip -= offset;
                break;
            }

            case OP_CALL: {
                int argCount = READ_BYTE();

                if (!callValue(peek(argCount), argCount)) {
                    return INTERPRET_RUNTIME_ERROR;
                }

                break;
            }

            case OP_RETURN: {
                Value result = pop();

                CallFrame finishedFrame = currentFrame();
                frames.pop_back();

                if (frames.empty()) {
                    pop();
                    return INTERPRET_OK;
                }

                stack.resize(finishedFrame.slotStart);
                push(result);

                break;
            }

            default:
                std::cout << "Unknown opcode "
                          << static_cast<int>(instruction)
                          << "\n";
                return INTERPRET_RUNTIME_ERROR;
        }
    }

#undef READ_BYTE
#undef READ_SHORT
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

bool VM::call(ObjFunction* function, int argCount) {
    if (argCount != function->arity) {
        runtimeError("Expected %d arguments but got %d.",
                     function->arity, argCount);
        return false;
    }

    if (static_cast<int>(frames.size()) == FRAMES_MAX) {
        runtimeError("Stack overflow.");
        return false;
    }

    CallFrame frame;
    frame.function = function;
    frame.ip = 0;
    frame.slotStart = static_cast<int>(stack.size()) - argCount - 1;

    frames.push_back(frame);

    return true;
}

bool VM::callValue(Value callee, int argCount) {
    if (IS_OBJ(callee)) {
        switch (OBJ_TYPE(callee)) {
            case OBJ_FUNCTION:
                return call(AS_FUNCTION(callee), argCount);

            case OBJ_NATIVE: {
                NativeFn native = AS_NATIVE(callee);

                Value result = native(argCount, stack.data() + stack.size() - argCount);

                stack.resize(stack.size() - argCount - 1);
                push(result);

                return true;
            }

            default:
                break;
        }
    }

    runtimeError("Can only call functions and classes.");
    return false;
}

uint8_t VM::readByte() {
    CallFrame& frame = currentFrame();
    return frame.function->chunk.getCode(frame.ip++);
}

uint16_t VM::readShort() {
    uint8_t high = readByte();
    uint8_t low = readByte();

    return static_cast<uint16_t>((high << 8) | low);
}

Value VM::readConstant() {
    CallFrame& frame = currentFrame();
    return frame.function->chunk.getConstant(readByte());
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

CallFrame& VM::currentFrame() {
    return frames.back();
}

Value VM::getStackSlot(int slot) const {
    const CallFrame& frame = frames.back();
    return stack[frame.slotStart + slot];
}

void VM::setStackSlot(int slot, Value value) {
    const CallFrame& frame = frames.back();
    stack[frame.slotStart + slot] = value;
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

void VM::defineNative(const char* name, NativeFn function) {
    push(OBJ_VAL(copyString(name, static_cast<int>(std::strlen(name)))));
    push(OBJ_VAL(newNative(function)));

    Value nativeValue = peek(0);
    ObjString* nativeName = AS_STRING(peek(1));

    globals.set(nativeName, nativeValue);

    pop();
    pop();
}

void VM::runtimeError(const char* format, ...) {
    va_list args;
    va_start(args, format);
    std::vfprintf(stderr, format, args);
    va_end(args);

    std::fputs("\n", stderr);

    for (int i = static_cast<int>(frames.size()) - 1; i >= 0; i--) {
        CallFrame& frame = frames[i];
        ObjFunction* function = frame.function;

        int instruction = frame.ip - 1;
        int line = function->chunk.getLine(instruction);

        std::fprintf(stderr, "[line %d] in ", line);

        if (function->name == nullptr) {
            std::fprintf(stderr, "script\n");
        } else {
            std::fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    resetStack();
}
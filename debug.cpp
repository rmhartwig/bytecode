//
// Created by ryanh on 5/9/2026.
//
#include "debug.h"

#include "value.h"

#include <iomanip>
#include <iostream>

static int simpleInstruction(const char* name, int offset) {
    std::cout << name << "\n";
    return offset + 1;
}

static int byteInstruction(const char* name, const Chunk& chunk, int offset) {
    uint8_t slot = chunk.getCode(offset + 1);

    std::cout << std::left << std::setw(16) << name
              << std::right << std::setw(4) << static_cast<int>(slot)
              << "\n";

    return offset + 2;
}

static int jumpInstruction(const char* name, int sign, const Chunk& chunk, int offset) {
    uint16_t jump = static_cast<uint16_t>(chunk.getCode(offset + 1) << 8);
    jump |= chunk.getCode(offset + 2);

    std::cout << std::left << std::setw(16) << name
              << std::right << std::setw(4) << offset
              << " -> "
              << offset + 3 + sign * jump
              << "\n";

    return offset + 3;
}

static int constantInstruction(const char* name, const Chunk& chunk, int offset) {
    uint8_t constant = chunk.getCode(offset + 1);

    std::cout << std::left << std::setw(16) << name
              << std::right << std::setw(4) << static_cast<int>(constant)
              << " '";

    printValue(chunk.getConstant(constant));

    std::cout << "'\n";

    return offset + 2;
}

void disassembleChunk(const Chunk& chunk, const char* name) {
    std::cout << "== " << name << " ==\n";

    for (int offset = 0; offset < chunk.count();) {
        offset = disassembleInstruction(chunk, offset);
    }
}

int disassembleInstruction(const Chunk& chunk, int offset) {
    std::cout << std::setfill('0') << std::setw(4) << offset << " ";
    std::cout << std::setfill(' ');

    if (offset > 0 && chunk.getLine(offset) == chunk.getLine(offset - 1)) {
        std::cout << "   | ";
    } else {
        std::cout << std::setw(4) << chunk.getLine(offset) << " ";
    }

    uint8_t instruction = chunk.getCode(offset);

    switch (instruction) {
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);

        case OP_NIL:
            return simpleInstruction("OP_NIL", offset);

        case OP_TRUE:
            return simpleInstruction("OP_TRUE", offset);

        case OP_FALSE:
            return simpleInstruction("OP_FALSE", offset);

        case OP_POP:
            return simpleInstruction("OP_POP", offset);

        case OP_GET_LOCAL:
            return byteInstruction("OP_GET_LOCAL", chunk, offset);

        case OP_SET_LOCAL:
            return byteInstruction("OP_SET_LOCAL", chunk, offset);

        case OP_GET_GLOBAL:
            return constantInstruction("OP_GET_GLOBAL", chunk, offset);

        case OP_DEFINE_GLOBAL:
            return constantInstruction("OP_DEFINE_GLOBAL", chunk, offset);

        case OP_SET_GLOBAL:
            return constantInstruction("OP_SET_GLOBAL", chunk, offset);

        case OP_EQUAL:
            return simpleInstruction("OP_EQUAL", offset);

        case OP_GREATER:
            return simpleInstruction("OP_GREATER", offset);

        case OP_LESS:
            return simpleInstruction("OP_LESS", offset);

        case OP_ADD:
            return simpleInstruction("OP_ADD", offset);

        case OP_SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);

        case OP_MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);

        case OP_DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);

        case OP_NOT:
            return simpleInstruction("OP_NOT", offset);

        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);

        case OP_PRINT:
            return simpleInstruction("OP_PRINT", offset);

        case OP_JUMP:
            return jumpInstruction("OP_JUMP", 1, chunk, offset);

        case OP_JUMP_IF_FALSE:
            return jumpInstruction("OP_JUMP_IF_FALSE", 1, chunk, offset);

        case OP_LOOP:
            return jumpInstruction("OP_LOOP", -1, chunk, offset);

        case OP_CALL:
            return byteInstruction("OP_CALL", chunk, offset);

        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);

        default:
            std::cout << "Unknown opcode " << static_cast<int>(instruction) << "\n";
            return offset + 1;
    }
}
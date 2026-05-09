//
// Created by ryanh on 5/9/2026.
//
#include "debug.h"

#include <iomanip>
#include <iostream>

static int simpleInstruction(const char* name, int offset) {
    std::cout << name << "\n";
    return offset + 1;
}

static int constantInstruction(const char* name, const Chunk& chunk, int offset) {
    uint8_t constant = chunk.getCode(offset + 1);

    std::cout << std::left << std::setw(16) << name
              << std::right << std::setw(4) << static_cast<int>(constant)
              << " '"
              << chunk.getConstant(constant)
              << "'\n";

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

        case OP_ADD:
            return simpleInstruction("OP_ADD", offset);

        case OP_SUBTRACT:
            return simpleInstruction("OP_SUBTRACT", offset);

        case OP_MULTIPLY:
            return simpleInstruction("OP_MULTIPLY", offset);

        case OP_DIVIDE:
            return simpleInstruction("OP_DIVIDE", offset);

        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);

        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);

        default:
            std::cout << "Unknown opcode " << static_cast<int>(instruction) << "\n";
            return offset + 1;
    }
}
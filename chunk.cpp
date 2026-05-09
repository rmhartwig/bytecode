//
// Created by ryanh on 5/9/2026.
//
#include "chunk.h"

void Chunk::write(uint8_t byte, int line) {
    code.push_back(byte);
    lines.push_back(line);
}

int Chunk::addConstant(Value value) {
    constants.write(value);
    return constants.count() - 1;
}

int Chunk::count() const {
    return static_cast<int>(code.size());
}

uint8_t Chunk::getCode(int index) const {
    return code[index];
}

int Chunk::getLine(int index) const {
    return lines[index];
}

Value Chunk::getConstant(int index) const {
    return constants.get(index);
}

void Chunk::setCode(int index, uint8_t byte) {
    code[index] = byte;
}
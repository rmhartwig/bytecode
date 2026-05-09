#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[]) {
    Chunk chunk;

    int constant = chunk.addConstant(1.2);
    chunk.write(OP_CONSTANT, 123);
    chunk.write(static_cast<uint8_t>(constant), 123);

    constant = chunk.addConstant(3.4);
    chunk.write(OP_CONSTANT, 123);
    chunk.write(static_cast<uint8_t>(constant), 123);

    chunk.write(OP_ADD, 123);

    constant = chunk.addConstant(5.6);
    chunk.write(OP_CONSTANT, 123);
    chunk.write(static_cast<uint8_t>(constant), 123);

    chunk.write(OP_DIVIDE, 123);

    chunk.write(OP_NEGATE, 123);

    chunk.write(OP_RETURN, 123);

    disassembleChunk(chunk, "test chunk");

    VM vm;
    vm.interpret(chunk);

    return 0;
}
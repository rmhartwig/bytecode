#include "chunk.h"
#include "debug.h"

int main(int argc, const char* argv[]) {
    Chunk chunk;

    int constant = chunk.addConstant(1.2);

    chunk.write(OP_CONSTANT, 123);
    chunk.write(static_cast<uint8_t>(constant), 123);
    chunk.write(OP_RETURN, 123);

    disassembleChunk(chunk, "test chunk");

    return 0;
}
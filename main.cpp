#include "vm.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static void repl() {
    VM vm;
    std::string line;

    for (;;) {
        std::cout << "> ";

        if (!std::getline(std::cin, line)) {
            std::cout << "\n";
            break;
        }

        vm.interpret(line);
    }
}

static std::string readFile(const char* path) {
    std::ifstream file(path, std::ios::binary);

    if (!file) {
        std::cerr << "Could not open file \"" << path << "\".\n";
        std::exit(74);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    if (file.bad()) {
        std::cerr << "Could not read file \"" << path << "\".\n";
        std::exit(74);
    }

    return buffer.str();
}

static void runFile(const char* path) {
    VM vm;

    std::string source = readFile(path);
    InterpretResult result = vm.interpret(source);

    if (result == INTERPRET_COMPILE_ERROR) std::exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) std::exit(70);
}

int main(int argc, const char* argv[]) {
    if (argc == 1) {
        repl();
    } else if (argc == 2) {
        runFile(argv[1]);
    } else {
        std::cerr << "Usage: BytecodeVM [path]\n";
        std::exit(64);
    }

    return 0;
}
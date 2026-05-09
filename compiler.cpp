//
// Created by ryanh on 5/9/2026.
//
#include "compiler.h"

#include "scanner.h"

#include <iostream>

void compile(const std::string& source) {
    Scanner scanner(source);

    int line = -1;

    for (;;) {
        Token token = scanner.scanToken();

        if (token.line != line) {
            std::cout.width(4);
            std::cout << token.line << " ";
            line = token.line;
        } else {
            std::cout << "   | ";
        }

        std::cout.width(2);
        std::cout << static_cast<int>(token.type) << " '"
                  << std::string(token.start, token.length)
                  << "'\n";

        if (token.type == TOKEN_EOF) break;
    }
}
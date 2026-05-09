//
// Created by ryanh on 5/9/2026.
//
#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H

#include "chunk.h"
#include "object.h"

#include <string>

bool compile(const std::string& source, Chunk& chunk);

#endif
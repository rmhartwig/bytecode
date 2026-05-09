//
// Created by ryanh on 5/9/2026.
//
#ifndef CLOX_COMPILER_H
#define CLOX_COMPILER_H

#include "object.h"

#include <string>

ObjFunction* compile(const std::string& source);

#endif
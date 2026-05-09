//
// Created by ryanh on 5/9/2026.
//
#ifndef CLOX_OBJECT_H
#define CLOX_OBJECT_H

#include "chunk.h"
#include "common.h"
#include "value.h"

enum ObjType {
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_STRING
};

struct Obj {
    ObjType type;
    Obj* next;
};

struct ObjString;

struct ObjFunction {
    Obj obj;
    int arity = 0;
    Chunk chunk;
    ObjString* name = nullptr;
};

using NativeFn = Value (*)(int argCount, Value* args);

struct ObjNative {
    Obj obj;
    NativeFn function = nullptr;
};

struct ObjString {
    Obj obj;
    int length = 0;
    char* chars = nullptr;
    uint32_t hash = 0;
};

inline ObjType OBJ_TYPE(Value value) {
    return AS_OBJ(value)->type;
}

inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

inline bool IS_FUNCTION(Value value) {
    return isObjType(value, OBJ_FUNCTION);
}

inline bool IS_NATIVE(Value value) {
    return isObjType(value, OBJ_NATIVE);
}

inline bool IS_STRING(Value value) {
    return isObjType(value, OBJ_STRING);
}

inline ObjFunction* AS_FUNCTION(Value value) {
    return reinterpret_cast<ObjFunction*>(AS_OBJ(value));
}

inline NativeFn AS_NATIVE(Value value) {
    return reinterpret_cast<ObjNative*>(AS_OBJ(value))->function;
}

inline ObjString* AS_STRING(Value value) {
    return reinterpret_cast<ObjString*>(AS_OBJ(value));
}

inline char* AS_CSTRING(Value value) {
    return AS_STRING(value)->chars;
}

ObjFunction* newFunction();
ObjNative* newNative(NativeFn function);

ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);

void printObject(Value value);
void freeObjects();

#endif
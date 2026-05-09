//
// Created by ryanh on 5/9/2026.
//
#ifndef CLOX_OBJECT_H
#define CLOX_OBJECT_H

#include "common.h"
#include "value.h"

enum ObjType {
    OBJ_STRING
};

struct Obj {
    ObjType type;
    Obj* next;
};

struct ObjString {
    Obj obj;
    int length;
    char* chars;
    uint32_t hash;
};

inline ObjType OBJ_TYPE(Value value) {
    return AS_OBJ(value)->type;
}

inline bool isObjType(Value value, ObjType type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}

inline bool IS_STRING(Value value) {
    return isObjType(value, OBJ_STRING);
}

inline ObjString* AS_STRING(Value value) {
    return reinterpret_cast<ObjString*>(AS_OBJ(value));
}

inline char* AS_CSTRING(Value value) {
    return AS_STRING(value)->chars;
}

ObjString* takeString(char* chars, int length);
ObjString* copyString(const char* chars, int length);

void printObject(Value value);
void freeObjects();

#endif
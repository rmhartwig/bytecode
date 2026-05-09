//
// Created by ryanh on 5/9/2026.
//
#include "object.h"

#include "table.h"

#include <cstring>
#include <iostream>

static Obj* objects = nullptr;
static Table strings;

static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;

    for (int i = 0; i < length; i++) {
        hash ^= static_cast<uint8_t>(key[i]);
        hash *= 16777619;
    }

    return hash;
}

static Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = reinterpret_cast<Obj*>(operator new(size));

    object->type = type;
    object->next = objects;
    objects = object;

    return object;
}

static ObjString* allocateString(char* chars, int length, uint32_t hash) {
    ObjString* string = reinterpret_cast<ObjString*>(
        allocateObject(sizeof(ObjString), OBJ_STRING)
    );

    string->length = length;
    string->chars = chars;
    string->hash = hash;

    strings.set(string, NIL_VAL());

    return string;
}

ObjString* takeString(char* chars, int length) {
    uint32_t hash = hashString(chars, length);

    ObjString* interned = strings.findString(chars, length, hash);

    if (interned != nullptr) {
        delete[] chars;
        return interned;
    }

    return allocateString(chars, length, hash);
}

ObjString* copyString(const char* chars, int length) {
    uint32_t hash = hashString(chars, length);

    ObjString* interned = strings.findString(chars, length, hash);

    if (interned != nullptr) {
        return interned;
    }

    char* heapChars = new char[length + 1];

    std::memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length, hash);
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            std::cout << AS_CSTRING(value);
            break;
    }
}

static void freeObject(Obj* object) {
    switch (object->type) {
        case OBJ_STRING: {
            ObjString* string = reinterpret_cast<ObjString*>(object);

            delete[] string->chars;
            operator delete(string);

            break;
        }
    }
}

void freeObjects() {
    strings.clear();

    Obj* object = objects;

    while (object != nullptr) {
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }

    objects = nullptr;
}
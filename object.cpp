//
// Created by ryanh on 5/9/2026.
//
#include "object.h"

#include <cstring>
#include <iostream>

static Obj* objects = nullptr;

static Obj* allocateObject(size_t size, ObjType type) {
    Obj* object = reinterpret_cast<Obj*>(operator new(size));

    object->type = type;
    object->next = objects;
    objects = object;

    return object;
}

static ObjString* allocateString(char* chars, int length) {
    ObjString* string = reinterpret_cast<ObjString*>(
        allocateObject(sizeof(ObjString), OBJ_STRING)
    );

    string->length = length;
    string->chars = chars;

    return string;
}

ObjString* takeString(char* chars, int length) {
    return allocateString(chars, length);
}

ObjString* copyString(const char* chars, int length) {
    char* heapChars = new char[length + 1];

    std::memcpy(heapChars, chars, length);
    heapChars[length] = '\0';

    return allocateString(heapChars, length);
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
    Obj* object = objects;

    while (object != nullptr) {
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }

    objects = nullptr;
}
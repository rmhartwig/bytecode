//
// Created by ryanh on 5/9/2026.
//
#include "object.h"

#include "table.h"

#include <cstring>
#include <iostream>

static Obj* objects = nullptr;
static Table strings;

static void linkObject(Obj* object, ObjType type) {
    object->type = type;
    object->next = objects;
    objects = object;
}

static uint32_t hashString(const char* key, int length) {
    uint32_t hash = 2166136261u;

    for (int i = 0; i < length; i++) {
        hash ^= static_cast<uint8_t>(key[i]);
        hash *= 16777619;
    }

    return hash;
}

ObjClosure* newClosure(ObjFunction* function) {
    ObjUpvalue** upvalues = nullptr;

    if (function->upvalueCount > 0) {
        upvalues = new ObjUpvalue*[function->upvalueCount];

        for (int i = 0; i < function->upvalueCount; i++) {
            upvalues[i] = nullptr;
        }
    }

    ObjClosure* closure = new ObjClosure();

    linkObject(&closure->obj, OBJ_CLOSURE);

    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalueCount = function->upvalueCount;

    return closure;
}

ObjFunction* newFunction() {
    ObjFunction* function = new ObjFunction();

    linkObject(&function->obj, OBJ_FUNCTION);

    function->arity = 0;
    function->upvalueCount = 0;
    function->name = nullptr;

    return function;
}

ObjNative* newNative(NativeFn function) {
    ObjNative* native = new ObjNative();

    linkObject(&native->obj, OBJ_NATIVE);

    native->function = function;

    return native;
}

ObjUpvalue* newUpvalue(Value* slot) {
    ObjUpvalue* upvalue = new ObjUpvalue();

    linkObject(&upvalue->obj, OBJ_UPVALUE);

    upvalue->location = slot;
    upvalue->closed = NIL_VAL();
    upvalue->next = nullptr;

    return upvalue;
}

static ObjString* allocateString(char* chars, int length, uint32_t hash) {
    ObjString* string = new ObjString();

    linkObject(&string->obj, OBJ_STRING);

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

static void printFunction(ObjFunction* function) {
    if (function->name == nullptr) {
        std::cout << "<script>";
        return;
    }

    std::cout << "<fn " << function->name->chars << ">";
}

void printObject(Value value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_CLOSURE:
            printFunction(AS_CLOSURE(value)->function);
            break;

        case OBJ_FUNCTION:
            printFunction(AS_FUNCTION(value));
            break;

        case OBJ_NATIVE:
            std::cout << "<native fn>";
            break;

        case OBJ_STRING:
            std::cout << AS_CSTRING(value);
            break;

        case OBJ_UPVALUE:
            std::cout << "upvalue";
            break;
    }
}

static void freeObject(Obj* object) {
    switch (object->type) {
        case OBJ_CLOSURE: {
            ObjClosure* closure = reinterpret_cast<ObjClosure*>(object);
            delete[] closure->upvalues;
            delete closure;
            break;
        }

        case OBJ_FUNCTION: {
            ObjFunction* function = reinterpret_cast<ObjFunction*>(object);
            delete function;
            break;
        }

        case OBJ_NATIVE: {
            ObjNative* native = reinterpret_cast<ObjNative*>(object);
            delete native;
            break;
        }

        case OBJ_STRING: {
            ObjString* string = reinterpret_cast<ObjString*>(object);

            delete[] string->chars;
            delete string;

            break;
        }

        case OBJ_UPVALUE: {
            ObjUpvalue* upvalue = reinterpret_cast<ObjUpvalue*>(object);
            delete upvalue;
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
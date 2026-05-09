//
// Created by ryanh on 5/9/2026.
//
#include "value.h"

#include "object.h"

#include <iostream>

void ValueArray::write(Value value) {
    values.push_back(value);
}

void ValueArray::printValue(Value value) const {
    ::printValue(value);
}

int ValueArray::count() const {
    return static_cast<int>(values.size());
}

Value ValueArray::get(int index) const {
    return values[index];
}

bool valuesEqual(Value a, Value b) {
    if (a.type != b.type) return false;

    switch (a.type) {
        case VAL_BOOL:
            return AS_BOOL(a) == AS_BOOL(b);

        case VAL_NIL:
            return true;

        case VAL_NUMBER:
            return AS_NUMBER(a) == AS_NUMBER(b);

        case VAL_OBJ:
            return AS_OBJ(a) == AS_OBJ(b);
    }

    return false;
}

void printValue(Value value) {
    switch (value.type) {
        case VAL_BOOL:
            std::cout << (AS_BOOL(value) ? "true" : "false");
            break;

        case VAL_NIL:
            std::cout << "nil";
            break;

        case VAL_NUMBER:
            std::cout << AS_NUMBER(value);
            break;

        case VAL_OBJ:
            printObject(value);
            break;
    }
}
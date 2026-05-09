//
// Created by ryanh on 5/9/2026.
//
#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include "common.h"

#include <vector>

enum ValueType {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER
};

struct Value {
    ValueType type;

    union {
        bool boolean;
        double number;
    } as;
};

inline bool IS_BOOL(Value value) {
    return value.type == VAL_BOOL;
}

inline bool IS_NIL(Value value) {
    return value.type == VAL_NIL;
}

inline bool IS_NUMBER(Value value) {
    return value.type == VAL_NUMBER;
}

inline bool AS_BOOL(Value value) {
    return value.as.boolean;
}

inline double AS_NUMBER(Value value) {
    return value.as.number;
}

inline Value BOOL_VAL(bool value) {
    Value result;
    result.type = VAL_BOOL;
    result.as.boolean = value;
    return result;
}

inline Value NIL_VAL() {
    Value result;
    result.type = VAL_NIL;
    result.as.number = 0;
    return result;
}

inline Value NUMBER_VAL(double value) {
    Value result;
    result.type = VAL_NUMBER;
    result.as.number = value;
    return result;
}

class ValueArray {
public:
    void write(Value value);
    void printValue(Value value) const;

    int count() const;
    Value get(int index) const;

private:
    std::vector<Value> values;
};

bool valuesEqual(Value a, Value b);
void printValue(Value value);

#endif
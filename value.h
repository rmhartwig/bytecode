//
// Created by ryanh on 5/9/2026.
//
#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include "common.h"

#include <vector>

struct Obj;
struct ObjString;

enum ValueType {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
    VAL_OBJ
};

struct Value {
    ValueType type;

    union {
        bool boolean;
        double number;
        Obj* obj;
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

inline bool IS_OBJ(Value value) {
    return value.type == VAL_OBJ;
}

inline bool AS_BOOL(Value value) {
    return value.as.boolean;
}

inline double AS_NUMBER(Value value) {
    return value.as.number;
}

inline Obj* AS_OBJ(Value value) {
    return value.as.obj;
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

template <typename T>
inline Value OBJ_VAL(T* object) {
    Value result;
    result.type = VAL_OBJ;
    result.as.obj = reinterpret_cast<Obj*>(object);
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
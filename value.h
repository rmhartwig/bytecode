//
// Created by ryanh on 5/9/2026.
//
#ifndef CLOX_VALUE_H
#define CLOX_VALUE_H

#include "common.h"

#include <vector>

using Value = double;

class ValueArray {
public:
    void write(Value value);
    void printValue(Value value) const;

    int count() const;
    Value get(int index) const;

private:
    std::vector<Value> values;
};

#endif
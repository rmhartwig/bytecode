//
// Created by ryanh on 5/9/2026.
//
#include "value.h"

#include <iostream>

void ValueArray::write(Value value) {
    values.push_back(value);
}

void ValueArray::printValue(Value value) const {
    std::cout << value;
}

int ValueArray::count() const {
    return static_cast<int>(values.size());
}

Value ValueArray::get(int index) const {
    return values[index];
}
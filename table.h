//
// Created by ryanh on 5/9/2026.
//
#ifndef CLOX_TABLE_H
#define CLOX_TABLE_H

#include "value.h"

#include <cstdint>
#include <vector>

struct ObjString;

struct Entry {
    ObjString* key = nullptr;
    Value value = NIL_VAL();
};

class Table {
public:
    bool get(ObjString* key, Value* value);
    bool set(ObjString* key, Value value);
    bool remove(ObjString* key);

    void addAll(Table& to);
    ObjString* findString(const char* chars, int length, uint32_t hash);

    void clear();

private:
    int count = 0;
    std::vector<Entry> entries;

    Entry* findEntry(std::vector<Entry>& bucketArray, ObjString* key);
    void adjustCapacity(int capacity);
};

#endif
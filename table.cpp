//
// Created by ryanh on 5/9/2026.
//
#include "table.h"

#include "object.h"

#include <cstring>

static constexpr double TABLE_MAX_LOAD = 0.75;

Entry* Table::findEntry(std::vector<Entry>& bucketArray, ObjString* key) {
    uint32_t index = key->hash % bucketArray.size();

    Entry* tombstone = nullptr;

    for (;;) {
        Entry* entry = &bucketArray[index];

        if (entry->key == nullptr) {
            if (IS_NIL(entry->value)) {
                // Empty entry.
                return tombstone != nullptr ? tombstone : entry;
            } else {
                // Tombstone.
                if (tombstone == nullptr) {
                    tombstone = entry;
                }
            }
        } else if (entry->key == key) {
            return entry;
        }

        index = (index + 1) % bucketArray.size();
    }
}

void Table::adjustCapacity(int capacity) {
    std::vector<Entry> newEntries;
    newEntries.resize(capacity);

    count = 0;

    for (Entry& oldEntry : entries) {
        if (oldEntry.key == nullptr) continue;

        Entry* dest = findEntry(newEntries, oldEntry.key);
        dest->key = oldEntry.key;
        dest->value = oldEntry.value;
        count++;
    }

    entries = std::move(newEntries);
}

bool Table::get(ObjString* key, Value* value) {
    if (count == 0) return false;

    Entry* entry = findEntry(entries, key);

    if (entry->key == nullptr) {
        return false;
    }

    *value = entry->value;
    return true;
}

bool Table::set(ObjString* key, Value value) {
    if (count + 1 > static_cast<int>(entries.size() * TABLE_MAX_LOAD)) {
        int capacity = entries.size() < 8
            ? 8
            : static_cast<int>(entries.size()) * 2;

        adjustCapacity(capacity);
    }

    Entry* entry = findEntry(entries, key);

    bool isNewKey = entry->key == nullptr;

    if (isNewKey && IS_NIL(entry->value)) {
        count++;
    }

    entry->key = key;
    entry->value = value;

    return isNewKey;
}

bool Table::remove(ObjString* key) {
    if (count == 0) return false;

    Entry* entry = findEntry(entries, key);

    if (entry->key == nullptr) {
        return false;
    }

    // Place a tombstone.
    entry->key = nullptr;
    entry->value = BOOL_VAL(true);

    return true;
}

void Table::addAll(Table& to) {
    for (Entry& entry : entries) {
        if (entry.key != nullptr) {
            to.set(entry.key, entry.value);
        }
    }
}

ObjString* Table::findString(const char* chars, int length, uint32_t hash) {
    if (count == 0) return nullptr;

    uint32_t index = hash % entries.size();

    for (;;) {
        Entry* entry = &entries[index];

        if (entry->key == nullptr) {
            // Stop if this is a real empty entry, not a tombstone.
            if (IS_NIL(entry->value)) {
                return nullptr;
            }
        } else if (entry->key->length == length &&
                   entry->key->hash == hash &&
                   std::memcmp(entry->key->chars, chars, length) == 0) {
            return entry->key;
        }

        index = (index + 1) % entries.size();
    }
}

void Table::clear() {
    entries.clear();
    count = 0;
}
#include "debug.h"
#include "table.h"
#include "lua_string.h"
#include "error.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

LUA_BOOL equals_ptr(void *a, void *b) {
    return a == b;
}

LUA_BOOL equals_str(void *a, void *b) {
    LUA_STR *str1 = (LUA_STR *) a;
    LUA_STR *str2 = (LUA_STR *) b;
    return str1->size == str2->size && 
        str1->hash == str2->hash && !strncmp(str1->str, str2->str, str1->size);
}

static ENTRY *find_entry_loc(ENTRY *entries, int capacity, hash_func hash, 
        equals_func eq, void *key) { 
    uint32_t index = hash(key) % capacity;
    ENTRY *tombstone = NULL;
    while (TRUE) {
        ENTRY *entry = &entries[index];
        if (entry->key != NULL && eq(entry->key, key)) {
            return entry;
        } else if (entry->key == NULL) {
            if (IS_NIL(entry->value))
                return (tombstone != NULL) ? tombstone : entry;
            else if (tombstone == NULL)
                tombstone = entry;
        }
        index = (index + 1) % capacity;
    }
    return NULL;
}


static ERR resize_table(TABLE *table) {
    int new_capacity = table->capacity * 2;
    ENTRY *new_entries = NULL;
    CHECK(SAFE_ALLOC(&new_entries, new_capacity * sizeof(ENTRY)) != ALLOC_ERR);
    for (int i = 0; i < new_capacity; i++) {
        new_entries[i].key = NULL;
        new_entries[i].value.type = NIL;
    }
    int new_size = 0;
    for (int i = 0; i < table->capacity; i++) {
        void *key = table->entries[i].key;
        if (key != NULL) {
            ENTRY *entry = find_entry_loc(new_entries, new_capacity, table->hash, table->eq, key);
            entry->key = key;
            entry->value = table->entries[i].value;
            ++new_size;
        }
    }
    SAFE_FREE(&table->entries);
    table->size = new_size;
    table->capacity = new_capacity;
    table->entries = new_entries;
    return SUCCESS;
lua_err:
    return ALLOC_ERR;
}

ERR init_table(TABLE *table, hash_func hash, equals_func eq) {
    table->size = 0;
    table->capacity = INIT_TABLE_CAPACITY;
    table->hash = hash;
    table->eq = eq;
    CHECK(SAFE_ALLOC(&table->entries, table->capacity * sizeof(ENTRY)) != ALLOC_ERR);
    for (int i = 0; i < table->capacity; i++) {
        table->entries[i].key = NULL;
        table->entries[i].value.type = NIL;
    }
    return SUCCESS;
lua_err:
    return FAIL;
}

int size_table(TABLE *table) {
    return table->size;
}

LUA_BOOL put_table(TABLE *table, void *key, void *value) {
    if (table->size >= TABLE_LOAD_FACTOR * table->capacity) {
        LOG_DEBUG("Resizing table...");
        if (resize_table(table) != SUCCESS)
            return FALSE;
    }
    if (key == NULL)
        return FALSE;
    ENTRY *entry = find_entry_loc(table->entries, table->capacity, table->hash, table->eq, key);
    LUA_BOOL new_key_inserted = entry->key == NULL && IS_NIL(entry->value);
    if (new_key_inserted)
        ++table->size;
    entry->key = key;
    make_lua_obj_cpy(value, &entry->value);
    return new_key_inserted;
}

void *get_table(TABLE *table, void *key) {
    if (key == NULL)
        return NULL;
    ENTRY *entry = find_entry_loc(table->entries, table->capacity, table->hash, table->eq, key);
    return (entry->key == NULL) ? NULL : &entry->value;
}

LUA_BOOL remove_table(TABLE *table, void *key) {
    ENTRY *entry = find_entry_loc(table->entries, table->capacity, table->hash, table->eq, key);
    LUA_BOOL removed = entry->key != NULL;
    entry->key = NULL;
    // no need to decrease size, tombstones still take up space (though we reuse them)
    return removed;
}

void *in_table(TABLE *table, void *key) {
    if (key == NULL)
        return NULL;
    ENTRY *entry = find_entry_loc(table->entries, table->capacity, table->hash, table->eq, key);
    return entry->key;
}

void destroy_table(TABLE *table) {
    SAFE_FREE(&table->entries);
    table->size = 0;
    table->capacity = 0;
    table->hash = NULL;
}

void destroy_table_and_keys(TABLE *table) {
    for (int i = 0; i < table->capacity; i++) {
        if (table->entries[i].key != NULL) {
            LUA_STR *str = (LUA_STR *) table->entries[i].key;
            destroy_lua_str(&str);
        }
    }
    destroy_table(table);
}


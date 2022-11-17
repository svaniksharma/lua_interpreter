#include "debug.h"
#include "table.h"
#include "error.h"

static ENTRY *find_entry_loc(ENTRY *entries, int capacity, hash_func hash, void *key) {
    uint32_t index = hash(key) % capacity;
    ENTRY *tombstone = NULL;
    while (TRUE) {
        ENTRY *entry = &entries[index];
        if (entry->key == key) {
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
            ENTRY *entry = find_entry_loc(new_entries, new_capacity, table->hash, key);
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

ERR init_table(TABLE *table, hash_func hash) {
    table->size = 0;
    table->capacity = INIT_TABLE_CAPACITY;
    table->hash = hash;
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
    ENTRY *entry = find_entry_loc(table->entries, table->capacity, table->hash, key);
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
    ENTRY *entry = find_entry_loc(table->entries, table->capacity, table->hash, key);
    return (entry->key == NULL) ? NULL : &entry->value;
}

LUA_OBJ *get_table_str(TABLE *table, char *key, int size) {
    uint32_t key_hash = str_hash_len(key, size);
    uint32_t index = key_hash % table->capacity;
    while (TRUE) {
        ENTRY *entry = &table->entries[index];
        LUA_STR *str = (LUA_STR *) entry->key;
        if (entry->key == NULL && IS_NIL(entry->value)) {
            return &entry->value;
        } else if (str->size == size && 
                str->hash == key_hash && !strncmp(str->str, key, size)) {
            return &entry->value;
        }
        index = (index + 1) % table->capacity;
    }
    return NULL; // unreachable
}

LUA_BOOL remove_table(TABLE *table, void *key) {
    ENTRY *entry = find_entry_loc(table->entries, table->capacity, table->hash, key);
    LUA_BOOL removed = entry->key != NULL;
    entry->key = NULL;
    // no need to decrease size, tombstones still take up space (though we reuse them)
    return removed;
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


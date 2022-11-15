#include "debug.h"
#include "structs.h"
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
            if (entry->value == NULL)
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
    if (SAFE_ALLOC(&new_entries, new_capacity * sizeof(ENTRY)) != SUCCESS) {
        REPORT_C_ERR("Couldn't allocate memory for resizing table");
        return ALLOC_ERR;
    }
    for (int i = 0; i < new_capacity; i++) {
        new_entries[i].key = NULL;
        new_entries[i].value = NULL;
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
}

ERR init_table(TABLE *table, hash_func hash) {
    table->size = 0;
    table->capacity = INIT_TABLE_CAPACITY;
    table->hash = hash;
    if (SAFE_ALLOC(&table->entries, table->capacity * sizeof(ENTRY)) != SUCCESS) {
        REPORT_C_ERR("Couldn't allocate memory for hash table");
        return ALLOC_ERR;
    }
    return SUCCESS;
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
    LUA_BOOL new_key_inserted = entry->key == NULL && entry->value == NULL;
    if (new_key_inserted)
        ++table->size;
    entry->key = key;
    entry->value = value;
    return new_key_inserted;
}

void *get_table(TABLE *table, void *key) {
    if (key == NULL)
        return NULL;
    ENTRY *entry = find_entry_loc(table->entries, table->capacity, table->hash, key);
    return (entry->key == NULL) ? NULL : entry->value;
}

// TODO: implement for string interning
char *get_table_str(TABLE *table, char *key) {
    return NULL;
}

LUA_BOOL remove_table(TABLE *table, void *key) {
    ENTRY *entry = find_entry_loc(table->entries, table->capacity, table->hash, key);
    LUA_BOOL removed = entry->key != NULL;
    entry->key = NULL;
    // no need to decrease size, tombstones still take up space (though we reuse them)
    return removed;
}

void destroy_table(TABLE *table) {
    table->size = 0;
    table->capacity = 0;
    table->hash = NULL;
    ENTRY *entries = table->entries;
    SAFE_FREE(&entries);
}

uint32_t str_hash(void *key) {
    char *str = (char *) key;
    uint32_t hash = 2166136261u;
    for (int i = 0; str[i] != '\0'; i++) {
        hash ^= (uint8_t) str[i];
        hash *= 16777619;
    }
    return hash;
} 


#include "database.h"
#include "hashtable.h"

#define INITIAL_CAPACITY 5
#define FNV_PRIME 0x10000001b3
#define FNV_OFFSET 0xcbf29ce48422325UL

uint64_t hash_fnv1(const char *key, size_t length) {
    uint64_t hash_value = FNV_OFFSET;
    for (int i = 0; i < length; i++) {
        hash_value ^= key[i];
        hash_value *= FNV_PRIME;
    }
    return hash_value;
}

database *db_create() {
    database *db = malloc(sizeof(*db));
    db->table = hash_table_create(INITIAL_CAPACITY, hash_fnv1, NULL);

    return db;
}

void db_free(database *db) {
    hash_table_destroy(db->table);
    free(db);
}

char **db_keys(database *db, int *num_keys) {
    return hash_table_keys(db->table, num_keys);
}

bool db_insert(database *db, const char *key,  char *data) {
    return hash_table_insert(db->table, key, data);
}

char *db_lookup(database *db, const char *key) {
    return hash_table_lookup(db->table, key);
}

void db_flushall(database *db) {
    hash_table_destroy(db->table);
    db->table = hash_table_create(INITIAL_CAPACITY, hash_fnv1, NULL);
}

int db_status(database *db, char *buff, size_t buff_size) {
    return hash_table_status(db->table, buff, buff_size);
}

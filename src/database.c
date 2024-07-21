#include "database.h"
#include "hashtable.h"
#include <unistd.h>

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

void wait_expanding(database *db) {
    while (db->expanding) {
        usleep(1000);
    }
}

void ht_resize(database *db, int new_capacity) {
    db->ht[1] = hash_table_create(new_capacity, hash_fnv1, NULL);
    db->expanding = true;

    int num_keys = 0;
    char **keys = hash_table_keys(db->ht[0], &num_keys);
    for (size_t i = 0; i < num_keys; i++) {
        char *val = (char *)hash_table_lookup(db->ht[0], keys[i]);
        hash_table_insert(db->ht[1], keys[i], val);
    }
    hash_table_destroy(db->ht[0]);
    db->ht[0] = db->ht[1];
    db->expanding = false;
    free(keys);
}

database *db_create() {
    database *db = malloc(sizeof(*db));
    db->expanding = false;
    db->ht[0] = hash_table_create(INITIAL_CAPACITY, hash_fnv1, NULL);

    return db;
}

void db_free(database *db) {
    wait_expanding(db);
    hash_table_destroy(db->ht[0]);
    free(db);
}

char **db_keys(database *db, int *num_keys) {
    wait_expanding(db);
    return hash_table_keys(db->ht[0], num_keys);
}

bool db_insert(database *db, const char *key,  char *data) {
    wait_expanding(db);
    int size = hash_table_size(db->ht[0]);
    int els = hash_table_elements(db->ht[0]);
    if (els == size) {
        ht_resize(db, size * 2);
    }
    return hash_table_insert(db->ht[0], key, data);
}

char *db_lookup(database *db, const char *key) {
    wait_expanding(db);
    return hash_table_lookup(db->ht[0], key);
}

void db_flushall(database *db) {
    wait_expanding(db);
    hash_table_destroy(db->ht[0]);
    db->ht[0] = hash_table_create(INITIAL_CAPACITY, hash_fnv1, NULL);
}

int db_status(database *db, char *buff, size_t buff_size) {
    wait_expanding(db);
    return hash_table_status(db->ht[0], buff, buff_size);
}

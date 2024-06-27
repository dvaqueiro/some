
#include "hashtable.h"
#include <string.h>
#include <stdio.h>

typedef struct entry {
    char *key;
    char *data;
    struct entry *next;
} entry;

struct _hash_table {
    uint32_t size;
    uint32_t num_elements;
    hashfunction *hash;
    cleanup_function *cleanup;
    entry **elements;
    uint64_t collisions;
};

static size_t hash_table_index(hash_table *ht, const char *key) {
    size_t result = ht->hash(key, strlen(key)) % ht->size;
    return result;
}

hash_table *hash_table_create(uint32_t size, hashfunction *hf, cleanup_function *cf) {
    hash_table *ht = malloc(sizeof(*ht));
    ht->size = size;
    ht->hash = hf;
    ht->num_elements = 0;
    ht->collisions = 0;
    if (cf == NULL) cf = free; //default cleanup function (free)
    ht->cleanup = cf;
    ht->elements = calloc(sizeof(entry *), ht->size);

    return ht;
}

void hash_table_destroy(hash_table *ht) {
    for (uint32_t i = 0; i < ht->size; i++) {
        while (ht->elements[i] != NULL) {
            entry *tmp = ht->elements[i];
            ht->elements[i] = ht->elements[i]->next;
            free(tmp->key);
            ht->cleanup(tmp->data);
            free(tmp);
        }
    }
    free(ht->elements);
    free(ht);
}

int hash_table_status(hash_table *ht, char *buff, size_t buff_size) {
    int res_len = 0;

    res_len += snprintf(buff + res_len, buff_size - res_len, "hash_table size %u:\n", ht->size);
    res_len += snprintf(buff + res_len, buff_size - res_len, "hash_table num elements %u:\n", ht->num_elements);
    res_len += snprintf(buff + res_len, buff_size - res_len, "hash_table colisions %lu:\n", ht->collisions);
    for (uint32_t i = 0; i < ht->size; i++) {
        if (ht->elements[i] != NULL) {
            entry *tmp = ht->elements[i];
            res_len += snprintf(buff + res_len, buff_size - res_len, " [%i] (%s): \"%s\"", i, tmp->key, tmp->data);
            while (tmp != NULL) {
                tmp = tmp->next;
                if (tmp != NULL) {
                    res_len += snprintf(buff + res_len, buff_size - res_len, "->(%s): \"%s\"", tmp->key, tmp->data);
                }
            }
            res_len += snprintf(buff + res_len, buff_size - res_len, "\n");
        }
    }

    return res_len;
}

char **hash_table_keys(hash_table *ht, int *num_keys) {
    char **keys = malloc(sizeof(char *) * ht->num_elements);
    for (uint32_t i = 0; i < ht->size; i++) {
        if (ht->elements[i] != NULL) {
            entry *tmp = ht->elements[i];
            keys[(*num_keys)++] = tmp->key;
            while (tmp != NULL) {
                tmp = tmp->next;
                if (tmp != NULL) {
                    keys[(*num_keys)++] = tmp->key;
                }
            }
        }
    }
    return keys;
}

bool hash_table_insert(hash_table *ht, const char *key, char *data) {
    //No null key or data
    if (key == NULL || data == NULL) {
        return false;
    }
    size_t index = hash_table_index(ht, key);

    //Update entry
    if (hash_table_lookup(ht, key) != NULL) {
        ht->num_elements--;
        hash_table_delete(ht, key);
    }

    //Create new entry
    entry *e = malloc(sizeof(*e));
    e->data = strdup(data);
    e->key = strdup(key);

    //insert entry on hashtable
    if (ht->elements[index]) {
        ht->collisions++;
    }
    e->next = ht->elements[index];
    ht->num_elements++;
    ht->elements[index] = e;

    return true;
}

char *hash_table_lookup(hash_table *ht, const char *key) {
    if (key == NULL || ht == NULL) return false;
    size_t index = hash_table_index(ht, key);

    entry *tmp = ht->elements[index];
    while (tmp != NULL && strcmp(tmp->key, key) != 0) {
        tmp = tmp->next;
    }
    if (tmp == NULL) return NULL;

    return tmp->data;
}

char *hash_table_delete(hash_table *ht, const char *key) {
    if (key == NULL || ht == NULL) return false;
    size_t index = hash_table_index(ht, key);

    entry *tmp = ht->elements[index];
    entry *prev = NULL;
    while (tmp != NULL && strcmp(tmp->key, key) != 0) {
        prev = tmp;
        tmp = tmp->next;
    }
    if (tmp == NULL) return NULL;

    if (prev == NULL) {
        //deleting the head of the list
        ht->elements[index] = tmp->next;
    } else {
        //deleting from somewhere not the head
        prev->next = tmp->next;
    }
    ht->num_elements--;
    char *result = tmp->data;
    free(tmp);

    return result;
}


uint64_t hash_table_collisions(hash_table *ht) {
    return ht->collisions;
}

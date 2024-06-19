
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
    ht->collisions = 0;
    if (cf == NULL) cf = free; //default cleanup function (free)
    ht->cleanup = cf;
    ht->elements = calloc(sizeof(entry *), ht->size);

    return ht;
}

void hash_table_destroy(hash_table *ht) {
    //TODO: what about individual elements
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

void hash_table_print(hash_table *ht) {
    for (uint32_t i = 0; i < ht->size; i++) {
        if (ht->elements[i] == NULL) {
            //printf("\t%i\t---\n", 1);
        } else {
            entry *tmp = ht->elements[i];
            printf(" [%i] (%s): \"%s\"", i, tmp->key, tmp->data);
            while (tmp != NULL) {
                tmp = tmp->next;
                if (tmp != NULL) {
                    printf("->(%s): \"%s\"", tmp->key, tmp->data);
                }
            }
            printf("\n");
        }
    }
}

bool hash_table_insert(hash_table *ht, const char *key, char *data) {
    //No null key or data
    if (key == NULL || data == NULL) {
        return false;
    }
    size_t index = hash_table_index(ht, key);

    //Update entry
    if (hash_table_lookup(ht, key) != NULL) {
        hash_table_delete(ht, key);
    }

    //Create new entry
    entry *e = malloc(sizeof(*e));
    e->data = strdup(data);
    //e->key = malloc(strlen(key) + 1);
    //strcpy(e->key, key);
    e->key = strdup(key);

    //insert entry on hashtable
    if (ht->elements[index]) {
        ht->collisions++;
    }
    e->next = ht->elements[index];
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
    char *result = tmp->data;
    free(tmp);

    return result;
}


uint64_t hash_table_collisions(hash_table *ht) {
    return ht->collisions;
}

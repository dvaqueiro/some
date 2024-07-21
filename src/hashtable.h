#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

typedef uint64_t hashfunction(const char*, size_t);
typedef void cleanup_function(void*);
typedef struct _hash_table hash_table;

hash_table *hash_table_create(uint32_t size, hashfunction *hf, cleanup_function *cf);
void hash_table_destroy(hash_table *ht);
int hash_table_status(hash_table *ht, char *buff, size_t buff_size);
bool hash_table_insert(hash_table *ht, const char *key,  char *data);
char *hash_table_lookup(hash_table *ht, const char *key);
char *hash_table_delete(hash_table *ht, const char *key);
char **hash_table_keys(hash_table *ht, int *num_keys);

#endif // !HASHTABLE_H

#ifndef DATABASE_H
#define DATABASE_H

#include "hashtable.h"

typedef struct database {
    int expanding;
    hash_table *ht[2];
} database;

database *db_create();
void db_free(database *db);
char **db_keys(database *db, int *num_keys);
bool db_insert(database *db, const char *key,  char *data);
char *db_lookup(database *db, const char *key);
void db_flushall(database *db);
int db_status(database *db, char *buff, size_t buff_size);

#endif // !DATABASE_H

#ifndef __KVDB_H__
#define __KVDB_H__

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DEBUG
#include "debug.h"

struct kvdb {
  const char *filename;
  char journal[64];
  sem_t sem;
  int fd;
  int jd;
};
typedef struct kvdb kvdb_t;

int kvdb_open(kvdb_t *db, const char *filename);
int kvdb_close(kvdb_t *db);
int kvdb_put(kvdb_t *db, const char *key, const char *value);
char *kvdb_get(kvdb_t *db, const char *key);

int journal_write(kvdb_t *db, const char *key, const char *value);
int journal_check(kvdb_t *db, bool already_open);

#endif

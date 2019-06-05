#ifndef __KVDB_H__
#define __KVDB_H__

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define DEBUG
#include "debug.h"

#if defined __i386__
#define LLD "%032lld"
#elif defined __x86_64__
#define LLD "%032ld"
#endif

#define Byte sizeof(int8_t)
#define KB (1024 * Byte)
#define MB (1024 * Byte)

struct kvdb {
  int fd;
  const char *filename;
};
typedef struct kvdb kvdb_t;

int kvdb_open(kvdb_t *db, const char *filename);
int kvdb_close(kvdb_t *db);
int kvdb_put(kvdb_t *db, const char *key, const char *value);
char *kvdb_get(kvdb_t *db, const char *key);

#endif

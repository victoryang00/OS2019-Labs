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
#define MB (1024 * KB)

#define SZ_KEYS  1 * MB
#define SZ_VALV 16 * MB
#define SZ_RSVD 17 * MB

#define OP_TYPE O_CREAT | O_RDWR
#define OP_PRIV S_IRWXU | S_IRWXG | S_IRWXO

#define ER_ALRD -1
#define ER_OPEN -2
#define ER_CLOS -3
#define ER_LOCK -4
#define ER_UNLK -5
#define RT_SUCC 0

struct kvdb {
  int fd;
  const char *filename;
};
typedef struct kvdb kvdb_t;

int kvdb_open(kvdb_t *db, const char *filename);
int kvdb_close(kvdb_t *db);
void kvdb_fsck(kvdb_t *db);
int kvdb_put(kvdb_t *db, const char *key, const char *value);
char *kvdb_get(kvdb_t *db, const char *key);

#endif

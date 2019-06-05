#include "kvdb.h"

int kvdb_open(kvdb_t *db, const char *filename) {
  db->filename = filename;
  db->fd = open(filename, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);

  if (db->fd == -1) return -1;
  Log("%s opened", db->filename);
  return 0;
}

int kvdb_close(kvdb_t *db) {
  Log("%s closed", db->filename);
  return (close(db->fd)) ? -1 : 0;
}

int kvdb_put(kvdb_t *db, const char *key, const char *value) {
  return 0;
}

char *kvdb_get(kvdb_t *db, const char *key) {
  char *value = NULL;
  if (flock(db->fd, LOCK_SH)) return NULL;

  if (flock(db->fd, LOCK_UN)) {
    if (value) free(value);
    return NULL;
  };
  return value;
}

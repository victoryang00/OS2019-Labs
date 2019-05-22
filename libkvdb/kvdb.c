#include "kvdb.h"

int kvdb_open(kvdb_t *db, const char *filename) {
  db->filename = filename;
  db->fd = open(filename, O_CREAT | O_RDWR);
  Log("%s opened", db->filename);
  return db->fd == -1 ? -1 : 0;
}

int kvdb_close(kvdb_t *db) {
  Log("%s closed", db->filename);
  return close(db->fd) ? -1 : 0;
}

int kvdb_put(kvdb_t *db, const char *key, const char *value) {
  if (flock(db->fd, LOCK_EX)) return -1;
  lseek(db->fd, 0, SEEK_END);
  char len1[16] = "";
  char len2[16] = "";
  sprintf(len1, "%08d", (int)strlen(key));
  sprintf(len2, "%08d", (int)strlen(value));
  write(db->fd, len1, strlen(len1));
  write(db->fd, len2, strlen(len2));
  write(db->fd, key, strlen(key));
  write(db->fd, value, strlen(value));
  write(db->fd, "\n", 1);
  if (flock(db->fd, LOCK_UN)) return -1;
  return 0;
}

char *kvdb_get(kvdb_t *db, const char *key) {
  char *value = NULL;
  if (flock(db->fd, LOCK_SH)) return NULL;

  off_t offset = 0;
  char buf[2048] = "";
  char key_read[1024] = "";
  char value_read[1024] = ""; 
  while (read(db->fd, buf, sizeof(buf))) {
    sscanf(buf, " %s %s", key_read, value_read);
    offset += strlen(key_read) + strlen(value_read) + 2;
    if (!strcmp(key_read, key)) {
      if (value) free(value);
      value = malloc(strlen(value_read) + 5);
      strcpy(value, value_read);
    }
  }

  if (flock(db->fd, LOCK_UN)) return NULL;
  return value;
}

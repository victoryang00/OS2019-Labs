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
  write(db->fd, "\n", 1);
  write(db->fd, key, strlen(key));
  write(db->fd, "\n", 1);
  write(db->fd, value, strlen(value));
  write(db->fd, "\n", 1);
  if (flock(db->fd, LOCK_UN)) return -1;
  return 0;
}

char *kvdb_get(kvdb_t *db, const char *key) {
  char *value = NULL;
  if (flock(db->fd, LOCK_SH)) return NULL;

  off_t offset = 0;
  int len1 = 0;
  int len2 = 0;
  char buf[512] = "";
  char key_read[256] = "";
  
  Log("reading from file");
  lseek(db->fd, 0, SEEK_SET);
  while (read(db->fd, buf, sizeof(buf))) {
    sscanf(buf, "%8d%8d%s", &len1, &len2, key_read);
    Log("read = (%d, %d, %s)", len1, len2, key_read);
    if (!strcmp(key_read, key)) {
      if (value) free(value);
      value = malloc(len2 + 1);
      lseek(db->fd, offset + 16 + len1 + 2, SEEK_SET);
      int x = read(db->fd, value, len2);
      Log("read bytes: %d", x);
      value[len2] = '\0';
      Log("value updated: %s", value);
    }
    offset += 16 + len1 + len2 + 3;
    lseek(db->fd, offset, SEEK_SET);
  }

  if (flock(db->fd, LOCK_UN)) {
    if (value) free(value);
    return NULL;
  };
  return value;
}

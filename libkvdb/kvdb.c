#include "kvdb.h"

int kvdb_open(kvdb_t *db, const char *filename) {
  db->filename = filename;
  sprintf(db->journal, "%s.journal", filename);
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
  
  lseek(db->fd, 0, SEEK_SET);
  while (read(db->fd, buf, sizeof(buf))) {
    sscanf(buf, "%8d%8d%s", &len1, &len2, key_read);
    //Log("read = (%d, %d, %s)", len1, len2, key_read);
    if (!strcmp(key_read, key)) {
      if (value) free(value);
      value = malloc(len2 + 1);
      lseek(db->fd, offset + 16 + len1 + 2, SEEK_SET);
      read(db->fd, value, len2);
      value[len2] = '\0';
      //Log("value updated: %s", value);
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

int journal_write(kvdb_t *db, const char *key, const char *value) {
  journal_check(db);
  
}

int journal_check(kvdb_t *db) {
  int jd = open(db->journal, O_RDWR);
  if (jd == -1) return 0;
  if (flock(jd, LOCK_EX)) return -1;

  char buf[32] = "";
  int is_valid = 0;
  read(jd, buf, sizeof(buf));
  sscanf(buf, "%d", &is_valid);
  if (!is_valid) {
    flock(jd, LOCK_UN);
    return 0;
  }

  if (flock(db->fd, LOCK_EX)) return -1;
  int offset = 0;
  int len1 = 0;
  int len2 = 0;
  lseek(jd, 2, SEEK_SET);
  read(jd, buf, sizeof(buf));
  sscanf(buf, "%d %d %d", &offset, &len1, &len2);

  char *key = malloc((size_t)len1);
  char *value = malloc((size_t)len2);
  lseek(jd, 20, SEEK_SET);
  read(jd, key, (size_t)len1);
  lseek(jd, 20 + len1 + 1, SEEK_SET);
  read(jd, value, (size_t)len2);

  char len1_str[16] = "";
  char len2_str[16] = "";
  sprintf(len1_str, "%08d", len1);
  sprintf(len2_str, "%08d", len2);
  write(db->fd, len1_str, strlen(len1_str));
  write(db->fd, len2_str, strlen(len2_str));
  write(db->fd, "\n", 1);
  write(db->fd, key, strlen(key));
  write(db->fd, "\n", 1);
  write(db->fd, value, strlen(value));
  write(db->fd, "\n", 1);
  
  flock(db->fd, LOCK_UN);

  lseek(jd, 0, SEEK_SET);
  write(jd, "0", 1);
  flock(jd, LOCK_UN);
  return 0;
}

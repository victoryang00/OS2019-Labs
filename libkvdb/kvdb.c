#include "kvdb.h"

int kvdb_open(kvdb_t *db, const char *filename) {
  db->filename = filename;
  sprintf(db->journal, "%s.journal", filename);
  db->fd = open(filename, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);
  db->jd = open(db->journal, O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO);

  if (db->fd == -1 || db->jd == -1) return -1;
  Log("%s opened", db->filename);
  return 0;
}

int kvdb_close(kvdb_t *db) {
  Log("%s closed", db->filename);
  return (close(db->fd) || close(db->jd)) ? -1 : 0;
}

int kvdb_put(kvdb_t *db, const char *key, const char *value) {
  journal_write(db, key, value);
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
  if (db->jd == -1) return -1;
  while (true) {
    if (flock(db->jd, LOCK_EX)) return -1;
    Log("before check");
    journal_check(db, true);
    Log("after  check");

    char buf[8] = "";
    lseek(db->jd, 0, SEEK_SET);
    read(db->jd, buf, sizeof(buf));
    if (buf[0] == '0') break;
    
    if (flock(db->jd, LOCK_UN)) return -1;
  }

  char buf[32] = "";
  sprintf(buf, "%08d %08d\n", (int)strlen(key), (int)strlen(value));
  lseek(db->jd, 2, SEEK_SET);
  write(db->jd, buf, sizeof(buf));

  lseek(db->jd, 20, SEEK_SET);
  write(db->jd, key, strlen(key));
  write(db->jd, "\n", 1);
  lseek(db->jd, 21 + strlen(key), SEEK_SET);
  write(db->jd, value, strlen(value));
  write(db->jd, "\n", 1);

  lseek(db->jd, 0, SEEK_SET);
  write(db->jd, "1", 1);

  journal_check(db, true);
  if (flock(db->jd, LOCK_UN)) return -1;
  return 0;
}

int journal_check(kvdb_t *db, bool already_open) {
  if (!already_open || flock(db->jd, LOCK_EX)) return -1;

  char buf[32] = "";
  int is_valid = 0;
  read(db->jd, buf, sizeof(buf));
  sscanf(buf, "%d", &is_valid);
  if (!is_valid) {
    if (!already_open) flock(db->jd, LOCK_UN);
    return 0;
  }

  if (flock(db->fd, LOCK_EX)) return -1;
  int offset = 0;
  int len1 = 0;
  int len2 = 0;
  lseek(db->jd, 2, SEEK_SET);
  read(db->jd, buf, sizeof(buf));
  sscanf(buf, "%d %d %d", &offset, &len1, &len2);

  char *key = malloc((size_t)len1);
  char *value = malloc((size_t)len2);
  lseek(db->jd, 20, SEEK_SET);
  read(db->jd, key, (size_t)len1);
  lseek(db->jd, 20 + len1 + 1, SEEK_SET);
  read(db->jd, value, (size_t)len2);

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

  lseek(db->jd, 0, SEEK_SET);
  write(db->jd, "0", 1);
  if (!already_open) flock(db->jd, LOCK_UN);
  return 0;
}
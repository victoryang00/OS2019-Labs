#include "kvdb.h"

inline void find_start(int fd) {
  lseek(fd, SZ_RSVD, SEEK_SET);
}

inline bool check_end(int fd) {
  lseek(fd, 0, SEEK_END);
  lseek(fd, -1, SEEK_CUR);
  char buf;
  read(fd, &buf, 1);
  return buf == '\n';
}

inline void find_end(int fd) {
  lseek(fd, 0, SEEK_END);
  lseek(fd, -1, SEEK_CUR);
  char buf;
  read(fd, &buf, 1);
  while (buf != '\n') {
    lseek(fd, -2, SEEK_CUR);
    read(fd, &buf, 1);
  }
}

int kvdb_open(kvdb_t *db, const char *filename) {
  db->filename = filename;
  db->fd = open(filename, OP_TYPE, OP_PRIV);
  if (db->fd == -1) return ER_OPEN;
  Log("%s opened", db->filename);

  off_t size = lseek(db->fd, 0, SEEK_END);
  boom();
  if ((size_t)size < SZ_RSVD) {
    char *buf = malloc(SZ_RSVD);
    buf[0] = 'N';
    buf[1] = '\n';
    buf[SZ_RSVD - 1] = '\n';
    lseek(db->fd, 0, SEEK_SET);
    write(db->fd, buf, SZ_RSVD);
    lseek(db->fd, 0, SEEK_SET);
    Log("%s initialized", db->filename);
  }
  return RT_SUCC;
}

int kvdb_close(kvdb_t *db) {
  if (db->fd == -1) return ER_ALRD;
  if (close(db->fd)) return ER_CLOS;
  db->fd = -1;
  Log("%s closed", db->filename);
  return RT_SUCC;
}

void kvdb_fsck(kvdb_t *db) {
  lseek(db->fd, 0, SEEK_SET);
  char *buf = malloc(SZ_RSVD);
  read(db->fd, buf, 2);
  if (buf[0] == 'Y') {
    CLog(FG_PURPLE, "fsck trys to update db");
    read(db->fd, buf, SZ_RSVD);
    char *key = malloc(SZ_KEYS);
    char *val = malloc(SZ_VALV);
    sscanf(buf, " %s %s", key, val);
    CLog(FG_PURPLE, "update pair: %s %s", key, val);
    
    find_end(db->fd);
    write(db->fd, key, strlen(key));
    write(db->fd, " ", 1);
    write(db->fd, val, strlen(val));
    write(db->fd, "\n", 1);
    // sync();
    free(key);
    free(val);

    lseek(db->fd, 0, SEEK_SET);
    write(db->fd, "N\n", 2);
    CLog(FG_PURPLE, "fsck finished");
  }
  free(buf);
}

int kvdb_put(kvdb_t *db, const char *key, const char *value) {
  if (flock(db->fd, LOCK_EX)) return ER_LOCK;
  kvdb_fsck(db);
  lseek(db->fd, 2, SEEK_SET);
  write(db->fd, key, strlen(key));
  write(db->fd, " ", 1);
  write(db->fd, value, strlen(value));
  write(db->fd, "\n", 1);
  lseek(db->fd, 0, SEEK_SET);
  write(db->fd, "Y\n", 2);
  // sync();
  kvdb_fsck(db);
  if (flock(db->fd, LOCK_UN)) return ER_UNLK;
  return 0;
}

char *kvdb_get(kvdb_t *db, const char *key) {
  if (flock(db->fd, LOCK_EX)) return NULL;
  char *buf = malloc(SZ_RSVD);
  char *rkey = malloc(SZ_KEYS);
  char *rval = malloc(SZ_VALV);
  char *ret = malloc(SZ_VALV);

  kvdb_fsck(db);
  find_start(db->fd);
  off_t offset = lseek(db->fd, 0, SEEK_CUR);
  while (read(db->fd, buf, SZ_RSVD)) {
    sscanf(buf, " %s %s", rkey, rval);
    CLog(FG_GREEN, "read key-value: %s %s", rkey, rval);
    if (!strcmp(key, rkey)) {
      CLog(FG_YELLOW, "updated val: %s", rval);
      strcpy(ret, rval);
    }
    offset += strlen(rkey) + strlen(rval) + 2;
    lseek(db->fd, offset, SEEK_SET);
  }

  free(buf);
  free(rkey);
  free(rval);
  if (flock(db->fd, LOCK_UN)) {
    free(ret);
    return NULL;
  }
  return ret;
}

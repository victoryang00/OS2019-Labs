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
#ifdef DEBUG
  srand((unsigned)time(0));
#endif
  db->filename = filename;
  db->fd = open(filename, OP_TYPE, OP_PRIV);
  if (db->fd == -1) return ER_OPEN;
  pthread_mutex_init(db->mutex, NULL);
  Log("%s opened", db->filename);

  off_t size = lseek(db->fd, 0, SEEK_END);
  boom("before initialize db");
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

int kvdb_lock(kvdb_t *db) {
  int mx = 0;
  if ((mx = pthread_mutex_lock(db->mutex))) {
    if (mx == EOWNERDEAD) {
      pthread_mutex_consistent(db->mutex);
    } else {
      return ER_MUTX;
    }
  }
  if (flock(db->fd, LOCK_EX)) return ER_LOCK;
  return RT_SUCC;
}

int kvdb_unlk(kvdb_t *db) {
  if (flock(db->fd, LOCK_UN)) return ER_UNLK;
  if (pthread_mutex_unlock(db->mutex)) return ER_MUTX;
  return RT_SUCC;
}

void kvdb_fsck(kvdb_t *db) {
  lseek(db->fd, 0, SEEK_SET);
  char *buf = malloc(SZ_RSVD);
  read(db->fd, buf, 2);
  boom("before fsck");
  if (buf[0] == 'Y') {
    CLog(FG_PURPLE, "fsck trys to update db");
    read(db->fd, buf, SZ_RSVD);
    char *key = malloc(SZ_KEYS);
    char *val = malloc(SZ_VALV);
    sscanf(buf, " %s %s", key, val);
    CLog(FG_PURPLE, "update pair: %s %s", key, val);
    
    find_end(db->fd);
    boom("before fsck write");
    write(db->fd, key, strlen(key));
    boom("fsck write-1");
    write(db->fd, " ", 1);
    boom("fsck write-2");
    write(db->fd, val, strlen(val));
    boom("fsck write-3");
    write(db->fd, "\n", 1);
    // sync();
    free(key);
    boom("fsck free-1");
    free(val);
    boom("fsck free-2");

    lseek(db->fd, 0, SEEK_SET);
    boom("fsck write-4");
    write(db->fd, "N\n", 2);
    CLog(FG_PURPLE, "fsck finished");
  }
  free(buf);
}

int kvdb_put(kvdb_t *db, const char *key, const char *value) {
  if (kvdb_lock(db)) return ER_LOCK;
  kvdb_fsck(db);
  lseek(db->fd, 2, SEEK_SET);
  boom("put-1");
  write(db->fd, key, strlen(key));
  boom("put-2");
  write(db->fd, " ", 1);
  boom("put-3");
  write(db->fd, value, strlen(value));
  boom("put-4");
  write(db->fd, "\n", 1);
  boom("put-5");
  lseek(db->fd, 0, SEEK_SET);
  boom("put-6");
  write(db->fd, "Y\n", 2);
  boom("put-7");
  // sync();
  kvdb_fsck(db);
  boom("put before unlk");
  if (kvdb_unlk(db)) return ER_UNLK;
  return 0;
}

char *kvdb_get(kvdb_t *db, const char *key) {
  if (kvdb_lock(db)) return NULL;
  char *buf = malloc(SZ_RSVD);
  char *rkey = malloc(SZ_KEYS);
  char *rval = malloc(SZ_VALV);
  char *ret = calloc(1, SZ_VALV);

  boom("get-1");
  kvdb_fsck(db);
  boom("get-2");
  find_start(db->fd);
  boom("get-3");
  off_t offset = lseek(db->fd, 0, SEEK_CUR);
  while (read(db->fd, buf, SZ_RSVD)) {
    sscanf(buf, " %s %s", rkey, rval);
    boom("get-4");
    CLog(FG_GREEN, "read key-value: %s %s", rkey, rval);
    if (!strcmp(key, rkey)) {
      CLog(FG_YELLOW, "updated val: %s", rval);
      strcpy(ret, rval);
    }
    offset += strlen(rkey) + strlen(rval) + 2;
    boom("get-5");
    lseek(db->fd, offset, SEEK_SET);
  }

  free(buf);
  free(rkey);
  free(rval);
  boom("get-6");
  if (kvdb_unlk(db)) {
    free(ret);
    return NULL;
  }
  return ret;
}

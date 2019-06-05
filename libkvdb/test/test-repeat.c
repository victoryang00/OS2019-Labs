#include "../kvdb.h"
#include <stdlib.h>

#define DEBUG
#include "../debug.h"

int main() {
  kvdb_t db;
  int ret = 0;
  
  Log(">>> Open");
  ret = kvdb_open(&db, "a.db");
  Assert(!ret, "returned %d", ret);

  while (true) {
    char cmd[16], key[128], val[128];
    printf(">>> ");
    scanf(" %s", cmd);
    if (!strcmp(cmd, "set")) {
      scanf(" %s %s", key, val);
      kvdb_put(&db, key, val);
    } else if (!strcmp(cmd, "get")) {
      scanf(" %s", key);
      char *ret = kvdb_get(&db, key);
      if (ret) {
        printf("<<< %s -> %s\n", key, ret);
        free(ret);
      } else {
        printf("<<< %s not found\n", key);
      }
    } else {
      printf("<<< invalid cmd\n");
    }
  }

  Log(">>> Close");
  ret = kvdb_close(&db);
  Assert(!ret, "returned %d", ret);
  return 0;
}

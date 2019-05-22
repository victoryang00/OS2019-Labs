#include "../kvdb.h"
#include <stdlib.h>

int main() {
  kvdb_t db;
  int ret = 0;
  const char *key = "operating-systems";
  char *value;
  
  ret = kvdb_open(&db, "a.db"); // BUG: should check for errors
  printf("ret is %d\n", ret);

  ret = kvdb_put(&db, key, "thard-pieces");
  printf("ret is %d\n", ret);

  value = kvdb_get(&db, key);

  ret = kvdb_close(&db);
  printf("ret is %d\n", ret);

  printf("[%s]: [%s]\n", key, value);
  free(value);
  return 0;
}

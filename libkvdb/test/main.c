#include "../kvdb.h"
#include <stdlib.h>

#define DEBUG
#include "../debug.h"

int main() {
  kvdb_t db;
  int ret = 0;
  const char *key = "operating-systems";
  char *value;
  
  Log(">>> Open");
  ret = kvdb_open(&db, "a.db"); // BUG: should check for errors

  Log(">>> Put");
  ret = kvdb_put(&db, key, "thard-pieces");

  Log(">>> Get");
  value = kvdb_get(&db, key);

  Log(">>> Close");
  ret = kvdb_close(&db);

  printf("[%s]: [%s]\n", key, value);
  free(value);
  return 0;
}

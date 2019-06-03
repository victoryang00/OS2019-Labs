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
  if (ret) Panic("returned %d", ret);

  Log(">>> Put");
  ret = kvdb_put(&db, key, "thard-pieces");
  if (ret) Panic("returned %d", ret);

  Log(">>> Get");
  value = kvdb_get(&db, key);
  if (!value) Panic("returned null");

  Log(">>> Close");
  ret = kvdb_close(&db);
  if (ret) Panic("returned %d", ret);

  printf("[%s]: [%s]\n", key, value);
  free(value);
  return 0;
}

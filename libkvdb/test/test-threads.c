#include "../kvdb.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>

#define DEBUG
#include "../debug.h"

int main() {
  kvdb_t db;
  const char *key = "operating-systems";
  char *value;
  
  kvdb_open(&db, "a.db");

  pid_t pid = fork();
  if (!pid) {

  }
  
  return 0;
}

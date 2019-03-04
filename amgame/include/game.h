#include <am.h>
#include <amdev.h>
#include <ctype.h>
#include <klib.h>
#include <stdbool.h>

#define SIDE 16

struct Point {
  int x, y;
} directions[4] = {
  {  0, -1 }, // up
  {  0,  1 }, // down
  { -1,  0 }, // left
  {  1,  0 }  // right
};

struct Snake {
  struct Point pos;
  struct Snake* next;
};

struct State {
  struct Snake* head;
  struct Snake* tail;

  int direction;
  bool insertMode;
};

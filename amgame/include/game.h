#include <am.h>
#include <amdev.h>
#include <klib.h>

#ifndef bool
#define bool int8_t
#define true 1
#define false 0
#endif

#define NR_SNAKE 32
#define CONST_FPS 2

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
  struct int next;
};

struct State {
  int width, height;
  int FPS, nextFrame;
  int keyCode;

  struct Point food;
  struct Snake snake[NR_SNAKE];
  struct int head;
  struct int tail;

  int direction;
  bool insertMode;
};

void drawSquare(struct Point pos, int size, uint32_t pixel) {
  assert(size < 100); // avoid memory boom
  uint32_t pixels[size][size];
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      pixels[i][j] = pixel;
    }
  }
  draw_rect(pixels, pos.x, pos.y, size, size);
}

#ifndef __GAME_H__
#define __GAME_H__

#include <am.h>
#include <amdev.h>
#include <klib.h>

#include <debug.h>

#ifndef bool
#define bool int8_t
#define true 1
#define false 0
#endif

#define GAME_ALL_FPS 24
#define GAME_TETRIS_FPS 2

struct Point {
  int x, y;
};

#include <tetris.h>

struct State {
  int width, height, blockSide;
  struct Point mainBias, tetrominosBias;
  int nextFrame, nextTetrisFrame;
  int score, keyCode;

  struct Tetromino tetromino;
  int nextTypes[NR_TETROMINOS];
};
extern struct State state;

void gameInit();
void clearScreen();
void drawSquare(struct Point, int, uint32_t);

#endif

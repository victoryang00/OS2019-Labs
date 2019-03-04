#pragma once
#ifndef __GAME_H__
#define __GAME_H__

#include <am.h>
#include <amdev.h>
#include <klib.h>

#ifndef bool
#define bool int8_t
#define true 1
#define false 0
#endif

#define CONST_FPS 2

struct Point {
  int x, y;
} directions[4] = {
  {  0, -1 }, // up
  {  0,  1 }, // down
  { -1,  0 }, // left
  {  1,  0 }  // right
};

#include <tetris.h>

struct State {
  int width, height;
  int FPS, nextFrame;
  int score, keyCode;

  struct Tetromino tetromino;
};
extern struct State state;

void drawSquare(struct Point, int, uint32_t);

#endif

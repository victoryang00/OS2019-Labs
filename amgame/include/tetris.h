#ifndef __TETRIS_H__
#define __TETRIS_H__

#include <game.h>

#define SCREEN_X 0
#define SCREEN_Y 0
#define SCREEN_H 26
#define SCREEN_W 15

enum uint32_t {
  //using material design colors
  EMPTY   = 0xffffff,
  RED     = 0xf44336,
  YELLOW  = 0xffeb3b,
  MAGENTA = 0x9c27b0,
  BLUE    = 0x2196f3,
  CYAN    = 0x00bcd4,
  GREEN   = 0x4caf50,
  ORANGE  = 0xff9800
};

struct Tetromino {
  struct Point pos;
  int type;
};

struct TetrominoType {
  struct Point d[4];
  uint32_t color;
  int prev, next;
};

struct KeyCodeMapping {
  int code;
  const char* name;
  struct Tetromino (*func) (struct Tetromino, bool);
  bool param;
};

void initTetris();
bool playTetris(int);
struct Tetromino newTetromino();
struct Tetromino moveTetromino(struct Tetromino, bool);
struct Tetromino fallTetromino(struct Tetromino, bool);
struct Tetromino spinTetromino(struct Tetromino, bool);
void saveTetromino(struct Tetromino);
void clearTetrominos();
bool checkTetromino(struct Tetromino);
bool checkPoint(struct Point);
bool checkPointPosition(struct Point);
bool checkRow(int);
void deleteRows(int, int);
void drawBlock(struct Point, uint32_t);
void drawTetrominos(struct Tetromino);

#endif

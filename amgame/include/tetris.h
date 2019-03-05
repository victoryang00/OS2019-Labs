#ifndef __TETRIS_H__
#define __TETRIS_H__

#include <game.h>

#define SCREEN_X 0
#define SCREEN_Y 0
#define SCREEN_H 26
#define SCREEN_W 13
#define SCREEN_BLOCK_SIDE 15

enum uint32_t {
  //TODO: find out the color codes
  EMPTY   = 0,
  RED     = 1234,
  YELLOW  = 2345,
  MAGENTA = 3456,
  BLUE    = 4567,
  CYAN    = 5678,
  GREEN   = 6789,
  ORANGE  = 7890
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
int checkTetromino(struct Tetromino);
int checkPoint(struct Point);
bool isLastRowFull();
void drawBlock(struct Point, uint32_t);
void drawTetrominos(struct Tetromino);

#endif

#include <game.h>

#define SCREEN_X 0
#define SCREEN_Y 0
#define SCREEN_H 26
#define SCREEN_W 13
#define SCREEN_BLOCK_SIDE 5

const int scores[5] = {
  0, 100, 200, 400, 800
};

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
const struct Tetromino TT_GAME_OVER = { {-2, -2}, 0 };
const struct Tetromino TT_TOUCH_GROUND = { {-1, -1}, 0 };
const size_t SZ_ROW = sizeof(struct Tetromino) * SCREEN_W;
const size_t SZ_MV_SCREEN = sizeof(struct Tetromino) * (SCREEN_H - 1);

struct TetrominoType {
  struct Point d[4];
  uint32_t color;
  int prev, next;
} const tetrominoTypes[20] = {
  {{{ 0,  0}, { 0,  0}, { 0,  0}, { 0,  0}},   EMPTY,  0,  0}, //  0-EM

  {{{-1,  0}, { 0,  0}, { 1,  0}, { 2,  0}},     RED,  2,  2}, //  1-I1
  {{{ 0, -1}, { 0,  0}, { 0,  1}, { 0,  2}},     RED,  1,  1}, //  2-I2

  {{{ 0,  0}, { 0, -1}, { 0, -2}, {-1,  0}},  YELLOW,  6,  4}, //  3-J1
  {{{ 0,  0}, {-1,  0}, {-2,  0}, { 0,  1}},  YELLOW,  3,  5}, //  4-J2
  {{{ 0,  0}, { 0,  1}, { 0,  2}, { 1,  0}},  YELLOW,  4,  6}, //  5-J3
  {{{ 0,  0}, { 1,  0}, { 2,  0}, { 0, -1}},  YELLOW,  5,  3}, //  6-J4

  {{{ 0,  0}, { 0, -1}, { 0, -2}, { 1,  0}}, MAGENTA, 10,  8}, //  7-L1
  {{{ 0,  0}, {-1,  0}, {-2,  0}, { 0, -1}}, MAGENTA,  7,  9}, //  8-L2
  {{{ 0,  0}, { 0,  1}, { 0,  2}, {-1,  0}}, MAGENTA,  8, 10}, //  9-L3
  {{{ 0,  0}, { 1,  0}, { 2,  0}, { 0,  1}}, MAGENTA,  9,  7}, // 10-L4
  
  {{{ 0,  0}, { 1,  0}, { 0,  1}, { 1,  1}},    BLUE, 11, 11}, // 11-O1

  {{{ 0,  0}, { 1,  0}, { 0,  1}, {-1,  1}},    CYAN, 13, 13}, // 12-S1
  {{{ 0,  0}, { 0, -1}, { 1,  0}, { 1,  1}},    CYAN, 12, 12}, // 13-S2

  {{{ 0,  0}, {-1,  0}, { 0,  1}, { 1,  0}},   GREEN, 17, 15}, // 14-T1
  {{{ 0,  0}, { 0, -1}, {-1,  0}, { 0,  1}},   GREEN, 14, 16}, // 15-T2
  {{{ 0,  0}, { 1,  0}, { 0, -1}, {-1,  0}},   GREEN, 15, 17}, // 16-T3
  {{{ 0,  0}, { 0,  1}, { 1,  0}, { 0, -1}},   GREEN, 16, 14}, // 17-T4

  {{{ 0,  0}, {-1,  0}, { 0,  1}, { 1,  1}},  ORANGE, 19, 19}, // 18-Z1
  {{{ 0,  0}, { 0, -1}, {-1,  0}, {-1,  1}},  ORANGE, 18, 18}  // 19-Z2
};
const int NR_TETROMINO_TYPES = sizeof(tetrominoTypes) / sizeof(struct TetrominoType);

void initTetris();
struct Tetromino newTetromino();
struct Tetromino moveTetromino(struct Tetromino, bool);
struct Tetromino fallTetromino(struct Tetromino, bool);
struct Tetromino spinTetromino(struct Tetromino, bool);
void saveTetromino(struct Tetromino);
void clearTetrominos();
int isTetrominoValid(struct Tetromino);
int isPointValid(struct Point);
bool isLastRowFull();
void drawBlock(struct Point, uint32_t);
void drawTetrominos(struct Tetromino);

struct KeyCodeMapping {
  int code;
  bool param;
  struct Tetromino (*func) (struct Tetromino, bool);
} const keyCodeMappings[4] = {
  //TODO: FIND OUT THE KEYCODES
  {01, false, &spinTetromino}, 
  {02,  true, &fallTetromino},
  {03,  true, &moveTetromino},
  {04, false, &moveTetromino}
};
const int NR_KEY_MAPPING = sizeof(keyCodeMappings) / sizeof(struct KeyCodeMapping);

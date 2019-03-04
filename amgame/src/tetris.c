/**
 * Tetris game
 */
#include <game.h>
static int screen[SCREEN_H][SCREEN_W];

const int scores[5] = {
  0, 100, 200, 400, 800
};

const struct Tetromino TT_GAME_OVER    = { {-2, -2}, 0 };
const struct Tetromino TT_TOUCH_GROUND = { {-1, -1}, 0 };
const size_t SZ_ROW = sizeof(struct Tetromino) * SCREEN_W;
const size_t SZ_MV_SCREEN = sizeof(struct Tetromino) * (SCREEN_H - 1);

const struct TetrominoType tetrominoTypes[20] = {
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

const struct KeyCodeMapping keyCodeMappings[4] = {
  {30, false, &spinTetromino}, // W
  {44, false, &fallTetromino}, // S
  {43,  true, &moveTetromino}, // A
  {45, false, &moveTetromino}  // D
};
const int NR_KEY_MAPPING = sizeof(keyCodeMappings) / sizeof(struct KeyCodeMapping);

void initTetris() {
  memset(screen, 0, sizeof(screen));
  state.tetromino = newTetromino();
}

bool playTetris(int keyCode) {
  fallTetromino(state.tetromino, false);
  for (int i = 0; i < NR_KEY_MAPPING; ++i) {
    if (keyCode == keyCodeMappings[i].code) {
      struct Tetromino T = keyCodeMappings[i].func(state.tetromino, keyCodeMappings[i].param);
      if (memcmp(&T, &TT_GAME_OVER, sizeof(struct Tetromino)) == 0) {
        printf("Game Over!\n");
        return false;
      }
      if (memcmp(&T, &TT_TOUCH_GROUND, sizeof(struct Tetromino)) == 0) {
        printf("Touch ground.\n");
        state.tetromino = newTetromino();
      } else {
        printf("Tetromino moved.\n");
        state.tetromino = T;
      }
      break;
    }
  }
  drawTetrominos(state.tetromino);
  printf("Tetris OK: (%d, %d)\n", state.tetromino.pos.x, state.tetromino.pos.y);
  return true;
}

struct Tetromino newTetromino() {
  struct Tetromino T;
  T.pos.x = SCREEN_W / 2;
  T.pos.y = 0;
  T.type = rand() % NR_TETROMINO_TYPES + 1;
  return T;
}

struct Tetromino moveTetromino(struct Tetromino oldT, bool movingLeft) {
  struct Tetromino newT = { {oldT.pos.x + movingLeft ? -1 : 1, oldT.pos.y}, oldT.type };
  return isTetrominoValid(newT) ? newT : oldT;
}

struct Tetromino fallTetromino(struct Tetromino originT, bool force) {
  struct Tetromino T = originT;
  if (force) {
    while (isTetrominoValid(T)) T.pos.y++;
  } else {
    struct Tetromino nextT = T;
    nextT.pos.y++;
    T = isTetrominoValid(nextT) ? nextT : T;
  }
  if (force || memcmp(&T, &originT, sizeof(struct Tetromino)) == 0) {
    if (isTetrominoValid(T) == -1) {
      return TT_GAME_OVER; // game over 
    } else {
      saveTetromino(T);
      return TT_TOUCH_GROUND; // touch ground
    }
  } else {
    return T;
  }
}

struct Tetromino spinTetromino(struct Tetromino oldT, bool clockwise) {
  struct Tetromino newT = oldT;
  newT.type = clockwise ? tetrominoTypes[oldT.type].prev : tetrominoTypes[oldT.type].next;
  return isTetrominoValid(newT) ? newT : oldT;
}

void saveTetromino(struct Tetromino T) {
  struct Point p;
  for (int i = 0; i < 4; ++i) {
    p.x = T.pos.x + tetrominoTypes[T.type].d[i].x;
    p.y = T.pos.y + tetrominoTypes[T.type].d[i].y;
    screen[p.y][p.x] = T.type;
  }
}

void clearTetrominos() {
  int combo = 0;
  while (isLastRowFull()) combo++;
  state.score += scores[combo];
}

int isTetrominoValid(struct Tetromino T) {
  int result = 1;
  struct Point p;
  for (int i = 0; i < 4; ++i) {
    p.x = T.pos.x + tetrominoTypes[T.type].d[i].x;
    p.y = T.pos.y + tetrominoTypes[T.type].d[i].y;
    int res = isPointValid(p);
    if (!res) return 0;
    if (res == -1) result = -1; // above screen
  }
  return result;
}

int isPointValid(struct Point p) {
  if (p.x < 0 || p.x >= SCREEN_W) return 0;
  else {
    if (p.y < 0) return -1; // above screen
    if (p.y >= SCREEN_H) return 0;
    return screen[p.y][p.x] != 0;
  }
}

bool isLastRowFull() {
  for (int j = 0; j < SCREEN_W; ++j) {
    if (!screen[SCREEN_H - 1][j]) return false;
  }
  int tmp[SCREEN_H][SCREEN_W] = {};
  memcpy(tmp, screen, SZ_MV_SCREEN);
  memcpy(screen + SZ_ROW, tmp, SZ_MV_SCREEN);
  return true;
}

void drawBlock(struct Point pos, uint32_t color) {
  if (isPointValid(pos) != 1) return;
  struct Point realPos = { 
    SCREEN_X + pos.x * SCREEN_BLOCK_SIDE, 
    SCREEN_Y + pos.y * SCREEN_BLOCK_SIDE
  };
  drawSquare(realPos, SCREEN_BLOCK_SIDE, color);
}

void drawTetrominos(struct Tetromino T) {
  struct Point p;
  for (int i = 0; i < SCREEN_H; ++i) {
    for (int j = 0; j < SCREEN_W; ++j) {
      p.x = i, p.y = j;
      drawBlock(p, tetrominoTypes[screen[i][j]].color);
    }
  } 
  for (int i = 0; i < 4; ++i) {
    p.x = T.pos.x + tetrominoTypes[T.type].d[i].x;
    p.y = T.pos.y + tetrominoTypes[T.type].d[i].y;
    drawBlock(p, tetrominoTypes[T.type].color);
  }
}

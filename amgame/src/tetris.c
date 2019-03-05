/**
 * Tetris game
 */
#include <game.h>

const int scores[5] = {
  0, 100, 200, 400, 800
};

const struct Tetromino TT_GAME_OVER    = { {-1, -1}, 0 };
const size_t SZ_ROW = sizeof(struct Tetromino) * SCREEN_W;

const struct TetrominoType tetrominoTypes[] = {
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

const struct KeyCodeMapping keyCodeMappings[] = {
  { 0, "auto", &fallTetromino, false}, // no key
  {30, "spin", &spinTetromino,  true}, // W
  {44, "fall", &fallTetromino,  true}, // S
  {43, "movl", &moveTetromino,  true}, // A
  {45, "movr", &moveTetromino, false}  // D
};
const int NR_KEY_MAPPING = sizeof(keyCodeMappings) / sizeof(struct KeyCodeMapping);

int screen[SCREEN_H][SCREEN_W];

void initTetris() {
  memset(screen, 0, sizeof(screen));
  state.tetromino = newTetromino();
}

bool playTetris(int keyCode) {
  struct Tetromino T = state.tetromino;
  for (int i = 0; i < NR_KEY_MAPPING; ++i) {
    if (keyCode == keyCodeMappings[i].code) {
      Log("[%s]", keyCodeMappings[i].name);
      T = keyCodeMappings[i].func(T, keyCodeMappings[i].param);
      break;
    }
  }

  if (memcmp(&T, &TT_GAME_OVER, sizeof(struct Tetromino)) == 0) {
    Log("Game over.");
    printf("Game Over!\n");
    printf("Your score: %d\n", state.score);
    return false;
  } else {
    state.tetromino = T;
  }

  drawTetrominos(state.tetromino);
  Log("Tetris OK: (%d, %d)\n", state.tetromino.pos.x, state.tetromino.pos.y);
  return true;
}

struct Tetromino newTetromino() {
  struct Tetromino T;
  T.pos.x = SCREEN_W / 2;
  T.pos.y = 2;
  T.type = rand() % (NR_TETROMINO_TYPES - 1) + 1;
  return T;
}

struct Tetromino moveTetromino(struct Tetromino oldT, bool movingLeft) {
  struct Tetromino newT = oldT;
  newT.pos.x += movingLeft ? -1 : 1;
  return checkTetromino(newT) ? newT : oldT;
}

struct Tetromino fallTetromino(struct Tetromino originT, bool force) {
  struct Tetromino T = originT;
  {
    bool nextIsValid = false;
    struct Tetromino nextT = T;
    do {
      nextT.pos.y++;
      nextIsValid = checkTetromino(nextT);
      T = nextIsValid ? nextT : T;
    } while (force && nextIsValid);
  }
  if (force || memcmp(&T, &originT, sizeof(struct Tetromino)) == 0) {
    if (!checkTetromino(T)) {
      return TT_GAME_OVER; // game over 
    } else {
      Log("Touch ground.");
      saveTetromino(T);
      clearTetrominos();
      Log("New tetromino type: (%d, %d, %d)",
        state.tetromino.pos.x, state.tetromino.pos.y, state.tetromino.type);
      return newTetromino(); // touch ground
    }
  } else {
    return T;
  }
}

struct Tetromino spinTetromino(struct Tetromino oldT, bool clockwise) {
  struct Tetromino newT = oldT;
  newT.type = clockwise ? tetrominoTypes[oldT.type].prev : tetrominoTypes[oldT.type].next;
  return checkTetromino(newT) ? newT : oldT;
}

void saveTetromino(struct Tetromino T) {
  struct Point p;
  for (int i = 0; i < 4; ++i) {
    p.x = T.pos.x + tetrominoTypes[T.type].d[i].x;
    p.y = T.pos.y + tetrominoTypes[T.type].d[i].y;
    assert(checkPoint(p));
    screen[p.y][p.x] = T.type;
  }
}

void clearTetrominos() {
  for (int i = SCREEN_H - 1; i >= 0; --i) {
    if (checkRow(i)) {
      int combo = 0;
      do {
        --i, ++combo;
      } while (checkRow(i));
      deleteRows(i, combo);
      state.score += scores[combo];
      Log("Scored: %d", scores[combo]);
      return;
    }
  }
}

bool checkTetromino(struct Tetromino T) {
  struct Point p;
  for (int i = 0; i < 4; ++i) {
    p.x = T.pos.x + tetrominoTypes[T.type].d[i].x;
    p.y = T.pos.y + tetrominoTypes[T.type].d[i].y;
    if (!checkPoint(p)) return false;
  }
  return true;
}

bool checkPoint(struct Point p) {
  return checkPointPosition(p)
    && (screen[p.y][p.x] == 0); // CAUTION
}

bool checkPointPosition(struct Point p) {
  return (p.x >= 0 && p.x < SCREEN_W)
    && (p.y >= 0 && p.y < SCREEN_H);
}

bool checkRow(int row) {
  for (int j = 0; j < SCREEN_W; ++j) {
    if (screen[row][j] == 0) return false;
  }
  return true;
}

void deleteRows(int row, int combo) {
  for (int i = row; i >= 0; --i) {
    memcpy(screen + (i + combo) * SZ_ROW, screen + i * SZ_ROW, SZ_ROW);
  }
  memset(screen, 0, combo * SZ_ROW);
}

void drawBlock(struct Point pos, uint32_t color) {
  if (!checkPointPosition(pos)) return;
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
      p.x = j, p.y = i; // CAUTION!
      drawBlock(p, tetrominoTypes[screen[i][j]].color);
    }
  } 
  for (int i = 0; i < 4; ++i) {
    p.x = T.pos.x + tetrominoTypes[T.type].d[i].x;
    p.y = T.pos.y + tetrominoTypes[T.type].d[i].y;
    drawBlock(p, tetrominoTypes[T.type].color);
  }
}

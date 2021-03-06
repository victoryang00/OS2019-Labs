/**
 * Tetris game
 */
#include <game.h>

const int scores[5] = {
  0, 500, 1000, 2000, 4000
};

const struct Point startPos = { SCREEN_W / 2, 2 };
const struct Tetromino TT_GAME_OVER = { {-1, -1}, 0 };

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
const int tetrominoGenres[8] = { 1, 2, 4, 4, 1, 2, 4, 2 };
const int NR_TETROMINO_GENRES = 7;
const int NR_TETROMINO_TYPES = 20;

const struct KeyCodeMapping keyCodeMappings[] = {
  { _KEY_NONE, "AUTO", &fallTetromino, false},
  {    _KEY_W, "SPIN", &spinTetromino,  true},
  {    _KEY_S, "FALL", &fallTetromino,  true},
  {    _KEY_A, "MOVL", &moveTetromino,  true},
  {    _KEY_D, "MOVR", &moveTetromino, false},
  {   _KEY_UP, "SPIN", &spinTetromino,  true},
  { _KEY_DOWN, "FALL", &fallTetromino,  true},
  { _KEY_LEFT, "MOVL", &moveTetromino,  true},
  {_KEY_RIGHT, "MOVR", &moveTetromino, false},
  {_KEY_SPACE, "SWAP", &swapTetromino, false}
};
const int NR_KEY_MAPPING = sizeof(keyCodeMappings) / sizeof(struct KeyCodeMapping);

int screen[SCREEN_H][SCREEN_W];

void initTetris() {
  memset(screen, 0, sizeof(screen));
  state.tetromino = newTetromino();
  for (int i = 0; i < NR_TETROMINOS; ++i) {
    state.nextTypes[i] = newTetrominoType();
  }
  drawGameSection(state.tetromino);
  drawRightSection();
}

bool playTetris(int keyCode) {
  state.score--; // operation penalty
  struct Tetromino T = state.tetromino;
  for (int i = 0; i < NR_KEY_MAPPING; ++i) {
    if (keyCode == keyCodeMappings[i].code) {
      if (keyCode != _KEY_NONE) Log("[%s]", keyCodeMappings[i].name);
      T = keyCodeMappings[i].func(T, keyCodeMappings[i].param);
      break;
    }
  }

  if (memcmp(&T, &TT_GAME_OVER, sizeof(struct Tetromino)) == 0) {
    printf("Game Over!\n");
    printf("Your score: %d\n", state.score);
    printGameOver();
    return false;
  } else {
    state.tetromino = T;
  }

  drawGameSection(state.tetromino);
  // Log("OK => (%d, %d)", state.tetromino.pos.x, state.tetromino.pos.y);
  return true;
}

int newTetrominoType() { 
  int sum = 0;
  int genre = rand() % (NR_TETROMINO_GENRES - 1) + 1;
  int type = rand() % tetrominoGenres[genre];
  for (int i = 0; i < genre; ++i) {
    sum += tetrominoGenres[i];
  }
  return sum + type;
}

struct Tetromino newTetromino() {
  struct Tetromino T;
  T.pos = startPos;
  T.type = newTetrominoType();
  return T;
}

struct Tetromino swapTetromino(struct Tetromino oldT, bool uselessBool) {
  if (uselessBool) Log("233333");
  struct Tetromino newT;
  newT.pos = startPos;
  newT.type = state.nextTypes[0];
  if (checkTetromino(newT)) {
    state.nextTypes[0] = oldT.type;
    drawRightSection();
    return newT;
  } else {
    return oldT;
  }
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
      Log("Game over");
      return TT_GAME_OVER; 
    } else {
      Log("Touch ground");
      saveTetromino(T);
      clearTetrominos();
      T.pos = startPos;
      T.type = state.nextTypes[0];
      for (int i = 0; i < NR_TETROMINOS - 1; ++i) state.nextTypes[i] = state.nextTypes[i + 1];
      state.nextTypes[NR_TETROMINOS - 1] = newTetrominoType();
      if (checkTetromino(T)) {
        drawRightSection();
        Log("New tetromino: ((%d, %d), %d)", T.pos.x, T.pos.y, T.type);
        return T;
      } else {
        return TT_GAME_OVER;
      }
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
  assert(row >= 0 && combo > 0 && row + combo < SCREEN_H);
  for (int i = row; i >= 0; --i) {
    memcpy(&screen[i + combo][0], &screen[i][0], SCREEN_W * sizeof(int));
  }
  memset(screen, 0, combo * SCREEN_W * sizeof(int));
}

void drawBlock(struct Point pos, uint32_t color) {
  if (!checkPointPosition(pos)) return;
  struct Point realPos = { 
    state.mainBias.x + pos.x * state.blockSide, 
    state.mainBias.y + pos.y * state.blockSide
  };
  drawSquare(realPos, state.blockSide, color);
}

void drawLine() {
  struct Point p;
  for (int i = 0; i < 2; ++i) {
    p.y = state.mainBias.y + 2 * state.blockSide - i;
    for (int j = 0; j < SCREEN_W * state.blockSide; ++j) {
      p.x = state.mainBias.x + j;
      drawSquare(p, 1, 0xff0000); // red
    }
  }
}

void drawGameSection(struct Tetromino T) {
  struct Point p;
  for (int i = 0; i < SCREEN_H; ++i) {
    for (int j = 0; j < SCREEN_W; ++j) {
      p.x = j, p.y = i; // CAUTION!
      drawBlock(p, tetrominoTypes[screen[i][j]].color);
    }
  } 

  drawLine();

  for (int i = 0; i < 4; ++i) {
    p.x = T.pos.x + tetrominoTypes[T.type].d[i].x;
    p.y = T.pos.y + tetrominoTypes[T.type].d[i].y;
    drawBlock(p, tetrominoTypes[T.type].color);
  }
}

void drawRightSection() {
  struct Point p;
  char scoreStr[16] = "";
  sprintf(scoreStr, "%d", state.score);

  clearScreen(state.tetrominosBias.x - 3 * state.blockSide, 0, (3 + 6 * NR_TETROMINOS) * state.blockSide, state.height);
  drawString("NEXT", state.tetrominosBias.x - state.blockSide, state.tetrominosBias.y);
  for (int i = 0; i < NR_TETROMINOS; ++i) {
    int type = state.nextTypes[i];
    for (int j = 0; j < 4; ++j) {
      p.x = state.tetrominosBias.x + tetrominoTypes[type].d[j].x * state.blockSide;
      p.y = state.tetrominosBias.y + 32 + (2 + 6 * i + tetrominoTypes[type].d[j].y) * state.blockSide;
      drawSquare(p, state.blockSide, tetrominoTypes[type].color);
    }
  }
  drawString("SCORE", state.tetrominosBias.x - state.blockSide, state.tetrominosBias.y + 32 + (2 + 6 * NR_TETROMINOS) * state.blockSide);
  drawString(scoreStr, state.tetrominosBias.x - state.blockSide, state.tetrominosBias.y + 32 + (2 + 6 * NR_TETROMINOS) * state.blockSide + 16);
}

void printGameOver() {
  drawString("Game Over", state.tetrominosBias.x - state.blockSide, state.tetrominosBias.y + 32 + (2 + 6 * NR_TETROMINOS) * state.blockSide + 2 * 16);
}

/**
 * Tetris game
 */
#include <game.h>

static struct State state;
static int screen[SCREEN_H][SCREEN_W];

void initTetris() {
  memset(screen, 0, sizeof(screen));
  newTetromino();
}

struct Tetromino newTetromino() {
  struct Tetromino T;
  T.pos.x = rand() % SCREEN_W;
  T.pos.y = 0;
  T.type = rand() % NR_TETROMINO_TYPES;
  return T;
}

bool playTetromino(int keyCode) {
  for (int i = 0; i < NR_KEY_MAPPING; ++i) {
    if (keyCode == keyCodeMappings[i].code) {
      struct Tetromino T = keyCodeMappings[i].func(state.tetromino, keyCodeMappings[i].param);
      if (memcmp(&T, &TT_GAME_OVER, sizeof(struct Tetromino)) == 0) return false;
      if (memcmp(&T, &TT_TOUCH_GROUND, sizeof(struct Tetromino)) == 0) {
        state.tetromino = newTetromino();
      } else {
        state.tetromino = T;
      }
    }
  } 
  drawTetrominos(state.tetromino);
  return true;
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
    if (isTetrominoValid(nextT)) T = nextT;
  }
  if (force || T == originT) {
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
  newT.type = clockwise ? tetrominoTypes[T.type].prev : tetrominoTypes[T.type].next;
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
    if (res == -1) result = -1; // above screen
  }
  return result;
}

int isPointValid(struct Point p) {
  if (p.x < 0 || p.x >= SCREEN_W) return 0;
  else {
    if (p.y < 0) return -1; // above screen
    if (p.y >= SCREEN_H) return 0;
    return !screen[p.y][p.x];
  }
}

bool isLastRowFull() {
  for (int j = 0; j < SCREEN_W; ++j) {
    if (!screen[SCREEN_H - 1][j]) return false;
  }
  int tmp[SCREEN_H][SCREEN_W] = {};
  memcpy(tmp, screen, SZ_MV_SCREEN);
  memcpy(screen + SZ_ROW, tmp, SZ_MV_SCREEN);
}

void drawBlock(struct Point pos, uint32_t color) {
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

/**
 * Tetris Game
 */

#include <game.h>

struct Point directions[4] = {
  {  0, -1 }, // up
  {  0,  1 }, // down
  { -1,  0 }, // left
  {  1,  0 }  // right
};

struct State state;

int main() {
  _ioe_init();
  gameInit();
  while (1) {
    while (uptime() < state.nextFrame) ;
    while ((state.keyCode = read_key()) != _KEY_NONE) {
      if (state.keyCode & 0x8000) continue; // ignore key down
      playTetris(state.keyCode);
    }
    if (uptime() >= state.nextTetrisFrame) {
      if (!playTetris(_KEY_NONE)) break;
      state.nextTetrisFrame += 1000 / GAME_TETRIS_FPS;
    }
    state.nextFrame += 1000 / GAME_ALL_FPS;
  }
  while ((state.keyCode = read_key()) == _KEY_NONE) ;
  return 0;
}

void gameInit() {
  state.width = screen_width();
  state.height = screen_height();
  state.blockSide = state.height / SCREEN_H;

  if (state.width / state.blockSide > 2 * SCREEN_W) {
    state.mainBias.x = (state.width - state.blockSide * SCREEN_W) / 4;
    state.mainBias.y = (state.height - state.blockSide * SCREEN_H) / 2;
  } else {
    state.mainBias.x = 0;
    state.mainBias.y = (state.height - state.blockSide * SCREEN_H) / 2;
  }
  state.tetrominosBias.x = state.mainBias.x + state.blockSide * (SCREEN_W + 3);
  state.tetrominosBias.y = 5 * state.blockSide;

  state.nextFrame = 0;
  state.nextTetrisFrame = 0;
  state.score = 0;
  state.keyCode = 0;

  printf("Press any key to start!\n");
  drawString("Press any key to start!", 0, 0);
  while ((state.keyCode = read_key()) == _KEY_NONE) ;
  clearScreen(0, 0, state.width, state.height);
  
  srand((int) uptime());
  initTetris();
}

void clearScreen(int x, int y, int w, int h) {
  uint32_t black = 0x000000;
  for (int i = 0; i < h; ++i) {
    for (int j = 0; j < w; ++j) {
      if ((x + j >= 0 && x + j < state.width)
          && (y + i >= 0 && y + i < state.height)) {
        draw_rect(&black, x + j, y + i, 1, 1); // CAUTION
      }
    }
  }
}

void drawSquare(struct Point pos, int size, uint32_t pixel) {
  assert(size < 20); // avoid memory boom
  uint32_t pixels[size][size];
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      pixels[i][j] = pixel;
    }
  }
  draw_rect((uint32_t*) pixels, pos.x, pos.y, size, size);
}

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
      if (!playTetris(_KEY_NONE)) return 0;
      state.nextTetrisFrame += 1000 / GAME_TETRIS_FPS;
    }
    state.nextFrame += 1000 / GAME_ALL_FPS;
  }
  return 0;
}

void gameInit() {
  state.width = screen_width();
  state.height = screen_height();
  state.blockSide = state.height / SCREEN_H;

  state.mainBias.x = (state.width - state.blockSide * SCREEN_W) / 3;
  state.mainBias.y = (state.height - state.blockSide * SCREEN_H) / 2;
  state.tetrominosBias.x = 2 * state.mainBias.x + state.blockSide * SCREEN_W;
  state.tetrominosBias.y = 4 * state.blockSide;

  state.nextFrame = 0;
  state.nextTetrisFrame = 0;
  state.score = 0;
  state.keyCode = 0;

  printf("Press any key to start!\n");
  while ((state.keyCode = read_key()) == _KEY_NONE) ;
  
  srand((int) uptime());
  initTetris();
}

void clearScreen() {
  uint32_t pixels[state.width * state.height];
  memset(pixels, 0xff, sizeof(pixels));
  draw_rect(pixels, 0, 0, state.width, state.height);
}

void drawSquare(struct Point pos, int size, uint32_t pixel) {
  assert(size < 100); // avoid memory boom
  uint32_t pixels[size][size];
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      pixels[i][j] = pixel;
    }
  }
  draw_rect((uint32_t*) pixels, pos.x, pos.y, size, size);
}

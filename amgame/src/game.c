/**
 * TITLE TBD.
 */

#include <game.h>

struct Point directions[4] = {
  {  0, -1 }, // up
  {  0,  1 }, // down
  { -1,  0 }, // left
  {  1,  0 }  // right
};
struct State state;

void gameInit();
void drawRect(int, int, int, int, uint32_t);

int main() {
  _ioe_init();
  gameInit();
  while (1) {
    while (uptime() < state.nextFrame) ;
    while ((state.keyCode = read_key()) != _KEY_NONE) {
      //handleKey();
      printf("%d\n", state.keyCode);
    }
    //gameProcess();
    //screenUpdate();
    //state.nextFrame += 1000 / state.FPS;
  }
  return 0;
}

void gameInit() {
  state.width = screen_width();
  state.height = screen_height();
  state.FPS = CONST_FPS;
  state.nextFrame = 0;
  state.score = 0;
  state.keyCode = 0;
}

void drawRect(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // WARNING: allocated on stack
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  draw_rect(pixels, x, y, w, h);
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

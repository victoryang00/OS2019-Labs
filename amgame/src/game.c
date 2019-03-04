/**
 * TITLE TBD.
 */

#include <game.h>

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

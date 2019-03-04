/**
 * TITLE TBD.
 */

#include <game.h>

static struct State state;

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
  int width = screen_width();
  int height = screen_height();
  state = {
    .width = width,
    .height = height,
    .FPS = CONST_FPS,
    .nextFrame = 0,
    .keyCode = 0,
    .food = { 
      rand() % width, 
      rand() % height
    },
    .snake = {},
    .head = 0,
    .tail = 0,
    .direction = rand() % 4,
    .insertMode = false
  } 
}

void drawRect(int x, int y, int w, int h, uint32_t color) {
  uint32_t pixels[w * h]; // WARNING: allocated on stack
  for (int i = 0; i < w * h; i++) {
    pixels[i] = color;
  }
  draw_rect(pixels, x, y, w, h);
}

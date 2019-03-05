#include "game.h"

extern struct State state;
extern char font8x8_basic[128][8];

char *itoa(int n)  {  
  static char s[64];
  int i = sizeof(s) - 1;
  do {
    s[--i] = n % 10 + '0';  
    n /= 10;
  } while(n > 0);  
  return &s[i];
}  

void drawCharacter(char ch, int x, int y) {
  int i, j;
  char *p = font8x8_basic[(int)ch];
  uint32_t black = 0x000000, white = 0xffffff;
  for (i = 0; i < 16; i ++) {
    for (j = 0; j < 16; j ++) { 
      if (x + j < state.width && y + i < state.height) {
        draw_rect((p[i >> 1] >> (j >> 1)) & 1 ? &white : &black, x + j, y + i, 1, 1);
      }
    }
  }
}

void drawString(const char *str, int x, int y) {
  while (*str) {
    drawCharacter(*str ++, x, y);
    if (x + 16 >= state.width) {
      y += 16; x = 0;
    } else {
      x += 16;
    }
  }
}

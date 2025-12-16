// @sylefeb 2022-01-10
// MIT license, see LICENSE_MIT in Silice repo root
// https://github.com/sylefeb/Silice/

#include "config.h"
#include "std.h"
#include "oled.h"
#include "display.h"
#include "printf.h"

#ifndef HWFBUFFER
#error This firmware needs HWFBUFFER defined
#endif

void main()
{
  // install putchar handler for printf
  f_putchar = display_putchar;
  // init display
  oled_init();
  oled_fullscreen();
  oled_clear(0);
  // gradient background
  int which = 0;
  while (1) {
    for (int c=0;c<3;++c) {
      *RGBSEL = c; // select channel
      for (int i = 0 ; i < 128; ++i) {
        for (int j = 0 ; j < 128; ++j) {
            display_framebuffer()[i + (j << 7)] = (c==which)?255:0;
        }
      }
    }
    which = (which+1)%3;
  }
}

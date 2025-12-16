// @sylefeb 2022-01-10
// MIT license, see LICENSE_MIT in Silice repo root
// https://github.com/sylefeb/Silice/

#include "config.h"
#include "sdcard.h"
#include "std.h"
#include "oled.h"
#include "display.h"
#include "printf.h"

#include "fat_io_lib/src/fat_filelib.h"

void clear_audio()
{
  // wait for a buffer swap (sync)
  int *addr = (int*)(*AUDIO);
  while (addr == (int*)(*AUDIO)) { }
  // go ahead
  for (int b=0 ; b<2 ; ++b) {
    // read directly in hardware buffer
    addr = (int*)(*AUDIO);
    // clear buffer
    memset(addr,0,512);
    // wait for buffer swap
    while (addr == (int*)(*AUDIO)) { }
  }
}

#define N_ITEMS 5

  const char *items[N_ITEMS] = {
    "the sound of silence",
    "sunday bloody sunday",
    "envole-moi",
    "boys don't cry",
    "blouson noir",
  };
#ifndef HWFBUFFER
#error This firmware needs HWFBUFFER defined
#endif
void main()
{
    // install putchar handler for printf
    f_putchar = display_putchar;

    oled_init();
    oled_fullscreen();
    // oled_pix(c,c,c);
    // oled_wait();
    memset(display_framebuffer(),0x00,128*128);
    display_refresh();

    display_set_cursor(0,0);
    display_set_front_back_color(255,0);
    printf("init ... ");
    display_refresh();

    // init sdcard
    sdcard_init();
    // initialise File IO Library
    fl_init();
    // attach media access functions to library
    while (fl_attach_media(sdcard_readsector, sdcard_writesector) != FAT_INIT_OK) {
        // try again, we need this
    }
    printf("done.\n");
    display_refresh();
    display_set_cursor(0,0);
    display_set_front_back_color(0,255);
    //printf("    ===== files =====    \n\n");
    // playing the track
    // -> open the file
    FL_FILE *f = fl_fopen("/music.raw","rb");
    FL_FILE *f2 = fl_fopen("/adulthair_meme.raw","rb");
    if (f2 == NULL) {
        printf("img.raw not found.\n");
        display_refresh();
    } else if (f == NULL) {
        // error, no file
        printf("music file not found.\n");
        display_refresh();
    } else {
        //printf("image found.\n");
        display_refresh();
        // read pixels in framebuffer
        fl_fread(display_framebuffer(),1,128*128,f2);
        // refresh display to show the image
        display_refresh();
        fl_fclose(f2);
        display_set_front_back_color(0,255);
        //printf("music file found.\n");
        display_refresh();
        display_set_front_back_color(255,0);
        //printf("playing ... ");
        display_refresh();
        int leds = 1;
        int dir  = 0;
        // Pause option
        int pause = 0;
        int button_pause = 0;
        // Menu option
        int selected = 0;
        // int i=255;
        // plays the entire file
        while (1) {
            // tackle button press
            int prev_button = button_pause;
            button_pause = *BUTTONS & (1<<2);
            int diff_button = button_pause && !prev_button;
            display_set_cursor(0,0);
            for (int i = 0; i < N_ITEMS; ++i) {
                if (i == selected) { // highlight selected
                    display_set_front_back_color(0,255);
                } else {
                    display_set_front_back_color(255,0);
                }
                printf("%d> %s\n",i,items[i]);
            }
            display_refresh();
            // read directly in hardware buffer
            if(!pause){
                int *addr = (int*)(*AUDIO);
                // (use 512 bytes reads to avoid extra copies inside fat_io_lib)
                int sz = fl_fread(addr,1,512,f);
                if (sz < 512) break; // reached end of file
                // wait for buffer swap
                while (addr == (int*)(*AUDIO)) {  }
            } else {
                clear_audio();
                display_set_cursor(0,50);
                printf("pause");

                display_refresh();
            }
            // light show!
            if (leds == 128 || leds == 1) { dir = 1-dir; }
            leds = dir ? leds << 1 : leds >> 1;
            *LEDS = leds;
            if (diff_button) {
                pause = !pause;
                f2 = fl_fopen("/adulthair_meme.raw","rb");
                fl_fread(display_framebuffer(),1,128*128,f2);
                display_refresh();
                fl_fclose(f2);
            }
            if (*BUTTONS & (1<<3)) {
                ++ selected;
            }
            if (*BUTTONS & (1<<4)) {
                -- selected;
            }
            /*if (*BUTTONS & (1<<5)) {
                i = i-(i>>3);
            }
            if (*BUTTONS & (1<<6)) {
                i = i+(i>>3) >= 255 ? 255:i+(i>>3);
                }*/
            // wrap around
            if (selected < 0) {
                selected = N_ITEMS - 1;
            }
            if (selected >= N_ITEMS) {
                selected = 0;
            }
    }
    // close
    fl_fclose(f);
  }

}

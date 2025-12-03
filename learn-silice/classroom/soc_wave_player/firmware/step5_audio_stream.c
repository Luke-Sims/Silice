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
        display_set_front_back_color(0,255);
        //printf("music file found.\n");
        display_refresh();
        display_set_front_back_color(255,0);
        //printf("playing ... ");
        display_refresh();
        int leds = 1;
        int dir  = 0;
        int pause = 0;
        int button = 0;
        // plays the entire file
        while (1) {
            // tackle button press
            int prev_button = button;
            button = *BUTTONS & (1<<3);
            int diff_button = button && !prev_button;
            // read directly in hardware buffer
            if(pause){
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
            }
    }
    // close
    fl_fclose(f);
    fl_fclose(f2);
  }

}

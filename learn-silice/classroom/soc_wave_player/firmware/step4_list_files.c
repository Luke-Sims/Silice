// @sylefeb 2022-01-10
// MIT license, see LICENSE_MIT in Silice repo root
// https://github.com/sylefeb/Silice/

#include "config.h"
#include "std.h"
#include "oled.h"
#include "display.h"
#include "printf.h"
#include "sdcard.h"

// include the fat32 library
#include "fat_io_lib/src/fat_filelib.h"

#define N_ITEMS 5
#define MAX_SIZE 64

char items[N_ITEMS][MAX_SIZE];

int main()
{
  // turn LEDs off
  *LEDS = 0;
  int selected = 0;
  int nb_item = 0;
  // install putchar handler for printf
  f_putchar = display_putchar;
  // init screen
  oled_init();
  oled_fullscreen();
  oled_clear(0);
  // init sdcard
  sdcard_init();
  // initialise File IO Library
  fl_init();
  // attach media access functions to library
  while (fl_attach_media(sdcard_readsector, sdcard_writesector) != FAT_INIT_OK) {
    // keep trying, we need this
  }
  // header
  display_set_cursor(0,0);
  display_set_front_back_color(0,255);
  printf("    ===== files =====    \n\n");
  display_refresh();
  display_set_front_back_color(255,0);
  // list files (see fl_listdirectory if at_io_lib/src/fat_filelib.c)
  const char *path = "/";
  FL_DIR dirstat;
  // FL_LOCK(&_fs);
  if (fl_opendir(path, &dirstat)) {
    struct fs_dir_ent dirent;
    while (fl_readdir(&dirstat, &dirent) == 0 && nb_item<MAX_SIZE) {
        if (!dirent.is_dir) {
            // print file name
            strncpy(items[nb_item],dirent.filename, 63);
            nb_item ++;
            //printf("%s\n", items[nb_item]);
        }
    }
    fl_closedir(&dirstat);
    }
  // FL_UNLOCK(&_fs);
  display_refresh();
  // enter menu
  while (1) {
      display_set_cursor(0,15);
      display_refresh();
      for (int i = 0; i < nb_item; ++i) {
          if (i == selected) { // highlight selected
              display_set_front_back_color(0,255);
          } else {
              display_set_front_back_color(255,0);
          }
          printf("%d> %s\n",i,items[i]);
      }

      // read buttons and update selection
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
          selected = nb_item - 1;
      }
        if (selected >= nb_item) {
            selected = 0;
        }

    }
  return 0;
}

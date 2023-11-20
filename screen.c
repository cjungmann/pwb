#include <stdlib.h>
#include <curses.h>
#include <term.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/ioctl.h>

#include "screen.h"
#include "terminal.h"
#include "get_keystroke.h"


const char* tis_bold_mode = NULL;
const char* tis_italic_mode = NULL;
const char* tis_reverse_mode = NULL;
const char* tis_normal_mode = NULL;

const char* tis_enter_standout_mode = NULL;
const char* tis_exit_standout_mode = NULL;

const char* tis_enter_ca_mode = NULL;
const char* tis_exit_ca_mode = NULL;

const char *tis_normal_cursor = NULL;
const char *tis_hide_cursor = NULL;
const char *tis_show_cursor = NULL;

const char* tis_clear_screen = NULL;
const char* tis_cpr = NULL;                // C-ursor P-osition R-eport: get cursor position
const char* tis_cursor_address = NULL;    // set the cursor position

bool tis_values_set = false;

struct caps_list {
   const char **value;
   const char *code;
};

void get_terminfo_values(void)
{
   struct caps_list clist[] = {
      {&tis_bold_mode, "md"},
      {&tis_italic_mode, "us"},
      {&tis_reverse_mode, "mr"},
      {&tis_normal_mode, "me"},

      {&tis_enter_standout_mode, "so"},
      {&tis_exit_standout_mode, "se"},
      {&tis_enter_ca_mode, "ti"},
      {&tis_exit_ca_mode, "te"},

      {&tis_hide_cursor, "vi"},
      {&tis_show_cursor, "vs"},
      {&tis_normal_cursor, "ve"},

      {&tis_cpr, "u7"},
      {&tis_cursor_address, "cm"},

      {&tis_clear_screen, "cl"},
      {NULL, NULL}
   };

   struct caps_list *ptr = clist;
   while (ptr->value)
   {
      get_termcap_string((const char**)ptr->value, ptr->code);
      ++ptr;
   }

   tis_values_set = true;

}

void screen_write_str(const char *str, int file_handle)
{
   int len = strlen(str);
   write(file_handle, str, len);
}


void reset_screen(void)
{
   assert(tis_values_set);
   screen_write_str(tis_clear_screen, STDIN_FILENO);
}

void set_bold_mode(void)
{
   assert(tis_values_set);
   screen_write_str(tis_bold_mode, STDIN_FILENO);
}

void set_italic_mode(void)
{
   assert(tis_values_set);
   screen_write_str(tis_italic_mode, STDIN_FILENO);
}

void set_reverse_mode(void)
{
   assert(tis_values_set);
   screen_write_str(tis_reverse_mode, STDIN_FILENO);
}

void set_normal_mode(void)
{
   assert(tis_values_set);
   screen_write_str(tis_normal_mode, STDIN_FILENO);
}

void start_standout_mode(void)
{
   assert(tis_values_set);
   screen_write_str(tis_enter_standout_mode, STDIN_FILENO);
}

void stop_standout_mode(void)
{
   assert(tis_values_set);
   screen_write_str(tis_exit_standout_mode, STDIN_FILENO);
}

void hide_cursor(void)
{
   assert(tis_values_set);
   screen_write_str(tis_hide_cursor, STDIN_FILENO);
}

void normal_cursor(void)
{
   assert(tis_values_set);
   screen_write_str(tis_normal_cursor, STDIN_FILENO);
}

void show_cursor(void)
{
   assert(tis_values_set);
   screen_write_str(tis_show_cursor, STDIN_FILENO);
}

void set_cursor_position(int row, int col)
{
   const char *str = tiparm(tis_cursor_address, row, col);
   if (str)
      screen_write_str(str, STDIN_FILENO);
}

void get_cursor_position(int *row, int *col)
{
   // Use raw mode to read response from terminal so
   // `scanf` won't wait for a newline.
   int filehandle = STDIN_FILENO;

   screen_write_str(tis_cpr, STDIN_FILENO);

   struct termios original;
   set_tios_raw_mode(&original, filehandle);
   set_tios_read_mode(filehandle, 1, 1);

   scanf("\x1b[%d;%dR", row, col);

   restore_tios_mode(&original, filehandle);
}

void get_screen_size(int *rows, int *cols)
{
   *rows = *cols = 0;
   struct winsize ws;
   int result = ioctl(STDIN_FILENO, TIOCGWINSZ, &ws);
   if (result == 0)
   {
      *rows = ws.ws_row;
      *cols = ws.ws_col;
   }
   else
   {
      int strow, stcol;
      get_cursor_position(&strow, &stcol);
      set_cursor_position(999,999);
      get_cursor_position(rows,cols);
      set_cursor_position(strow, stcol);
   }
}




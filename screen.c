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


void reset_screen(void)
{
   assert(tis_values_set);
   tputs(tis_clear_screen, 1, putchar);
}

void set_bold_mode(void)
{
   assert(tis_values_set);
   tputs(tis_bold_mode, 1, putchar);
}

void set_italic_mode(void)
{
   assert(tis_values_set);
   tputs(tis_italic_mode, 1, putchar);
}

void set_reverse_mode(void)
{
   assert(tis_values_set);
   tputs(tis_reverse_mode, 1, putchar);
}

void set_normal_mode(void)
{
   assert(tis_values_set);
   tputs(tis_normal_mode, 1, putchar);
}

void start_standout_mode(void)
{
   assert(tis_values_set);
   tputs(tis_enter_standout_mode, 1, putchar);
}

void stop_standout_mode(void)
{
   assert(tis_values_set);
   tputs(tis_exit_standout_mode, 1, putchar);
}

void hide_cursor(void)
{
   assert(tis_values_set);
   tputs(tis_hide_cursor, 1, putchar);
}

void normal_cursor(void)
{
   assert(tis_values_set);
   tputs(tis_normal_cursor, 1, putchar);
}

void show_cursor(void)
{
   assert(tis_values_set);
   tputs(tis_show_cursor, 1, putchar);
}

void set_cursor_position(int row, int col)
{
   const char *str = tiparm(tis_cursor_address, row, col);
   if (str)
      tputs(str, 1, putchar);
}

void get_cursor_position(int *row, int *col)
{
   tputs(tis_cpr, 1, putchar);

   // Use raw mode to read response from terminal so
   // `scanf` won't wait for a newline.
   int filehandle = STDIN_FILENO;
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




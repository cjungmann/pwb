#include <stdlib.h>
#include <curses.h>
#include <term.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/ioctl.h>

#include "export.h"
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

const char* tis_enter_keyboard_transmit_mode = NULL;
const char* tis_exit_keyboard_transmit_mode = NULL;

const char* tis_clear_screen = NULL;
const char* tis_cpr = NULL;                // C-ursor P-osition R-eport: get cursor position
const char* tis_cursor_address = NULL;    // set the cursor position

// Values used outside of screen.c
const char* tis_scroll_forward = NULL;
const char* tis_scroll_reverse = NULL;
const char* tis_change_scroll_region = NULL;

bool tis_values_set_flag = false;

struct caps_list {
   const char **value;
   const char *code;
};

bool tis_values_set(void) { return tis_values_set_flag; }

void screen_write_str(const char *str, int file_handle)
{
   int len = strlen(str);
   int bytes_read = write(file_handle, str, len);
   assert(bytes_read == len);
}

extern void get_terminfo_values(void) __attribute__((constructor));

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

      {&tis_enter_keyboard_transmit_mode, "ks"},
      {&tis_exit_keyboard_transmit_mode, "ke"},

      {&tis_cpr, "u7"},
      {&tis_cursor_address, "cm"},

      {&tis_clear_screen, "cl"},

      {&tis_scroll_forward, "sf"},
      {&tis_scroll_reverse, "sr"},
      {&tis_change_scroll_region, "cs"},

      {NULL, NULL}
   };

   struct caps_list *ptr = clist;
   while (ptr->value)
   {
      get_termcap_string((const char**)ptr->value, ptr->code);
      ++ptr;
   }

   tis_values_set_flag = true;

}

EXPORT void reset_screen(void)
{
   assert(tis_values_set());
   screen_write_str(tis_clear_screen, STDOUT_FILENO);
}

void pwb_enter_ca_mode(void)
{
   assert(tis_values_set());
   screen_write_str(tis_enter_ca_mode, STDOUT_FILENO);
}

void pwb_exit_ca_mode(void)
{
   assert(tis_values_set());
   screen_write_str(tis_exit_ca_mode, STDOUT_FILENO);
}

EXPORT void set_bold_mode(void)
{
   assert(tis_values_set());
   screen_write_str(tis_bold_mode, STDOUT_FILENO);
}

EXPORT void set_italic_mode(void)
{
   assert(tis_values_set());
   screen_write_str(tis_italic_mode, STDOUT_FILENO);
}

EXPORT void set_reverse_mode(void)
{
   assert(tis_values_set());
   screen_write_str(tis_reverse_mode, STDOUT_FILENO);
}

EXPORT void set_normal_mode(void)
{
   assert(tis_values_set());
   screen_write_str(tis_normal_mode, STDOUT_FILENO);
}

EXPORT void start_standout_mode(void)
{
   assert(tis_values_set());
   screen_write_str(tis_enter_standout_mode, STDOUT_FILENO);
}

EXPORT void stop_standout_mode(void)
{
   assert(tis_values_set());
   screen_write_str(tis_exit_standout_mode, STDOUT_FILENO);
}

EXPORT void hide_cursor(void)
{
   assert(tis_values_set());
   screen_write_str(tis_hide_cursor, STDOUT_FILENO);
}

EXPORT void normal_cursor(void)
{
   assert(tis_values_set());
   screen_write_str(tis_normal_cursor, STDOUT_FILENO);
}

EXPORT void show_cursor(void)
{
   assert(tis_values_set());
   screen_write_str(tis_show_cursor, STDOUT_FILENO);
}

void enter_keyboard_transmit_mode(void)
{
   assert(tis_values_set());
   screen_write_str(tis_enter_keyboard_transmit_mode, STDOUT_FILENO);
}

void exit_keyboard_transmit_mode(void)
{
   assert(tis_values_set());
   screen_write_str(tis_exit_keyboard_transmit_mode, STDOUT_FILENO);
}

void set_scroll_limits(int top, int bottom)
{
   assert(tis_values_set());
   screen_write_str(tiparm(tis_change_scroll_region, top, bottom), STDOUT_FILENO);
}

void clear_scroll_limits(void)
{
   assert(tis_values_set());
   int row, col;
   get_screen_size(&row, &col);
   screen_write_str(tiparm(tis_change_scroll_region, 0, row-1), STDOUT_FILENO);
}


void set_cursor_position(int row, int col)
{
   const char *str = tiparm(tis_cursor_address, row, col);
   if (str)
      screen_write_str(str, STDOUT_FILENO);
}


void get_cursor_position(int *row, int *col)
{
   // Use raw mode to read response from terminal so
   // `scanf` won't wait for a newline.
   int filehandle = STDOUT_FILENO;

   screen_write_str(tis_cpr, STDOUT_FILENO);

   struct termios original, raw;
   tcgetattr(filehandle, &original);
   raw = original;
   set_rawread_mode(&raw);
   tcsetattr(filehandle, TCSANOW, &raw);

   scanf("\x1b[%d;%dR", row, col);

   tcsetattr(filehandle, TCSAFLUSH, &original);
}

void get_screen_size(int *rows, int *cols)
{
   (*rows) = 0;
   (*cols) = 0;
   struct winsize ws;
   int result = ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
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




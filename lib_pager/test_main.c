#include <stdio.h>
#include <string.h>       // strlen()

#include <unistd.h>       // write()

#include <stdarg.h>       // for va_list, etc
#include <alloca.h>

#include <assert.h>
#include <sys/ioctl.h>    // for ioctl() in get_screen_size()

#include "pager.h"

const char *dsource[] = {
   "One",
   "Two",
   "Three",
   "Four",
   "Five",
   "Six",
   "Seven",
   "Eight",
   "Nine",
   "Ten"
};

typedef struct string_array_source SAS;

struct string_array_source {
   const char **source;
   int length;
};

void write_to_screen(const char *str)
{
   int len = strlen(str);
   int bytes_written = write(STDOUT_FILENO, str, len);
   assert(bytes_written = len);
}


int write_printf(const char *fmt, ...)
{
   static char s_buff[64];
   char *buff = NULL;
   int blen = sizeof(s_buff);

   va_list args;
   va_start(args, fmt);

   int len = vsnprintf(NULL, 0, fmt, args);

   if (len >= 64)
      buff = s_buff;
   else
   {
      blen = len + 1;
      buff = (char*)alloca(blen);
   }

   va_start(args, fmt);
   vsnprintf(buff, blen, fmt, args);

   write(STDOUT_FILENO, buff, len);

   va_end(args);

   return len;
}

/**
 * @brief Change settings to raw terminal io mode
 *
 * Use `tcgetattr` for current settings, the `cf_make_row` to modify,
 * then `tcsetattr` to use the raw settings.

 * Code borrowd from not-guaranteed C library function.
 * cfmakeraw().
 *
 * @param "tos"  pointer to struct with valid termios values
 *
 * @code(c)
 * struct termios original, raw;
 * tcgetattr(STDOUT_FILENO, &original);
 * // Preserve original settings with copy to modify: 
 * raw = original;
 * cf_make_raw(&raw);
 * tcsetattr(STDOUT_FILENO, &raw);
 * work_with_row();
 * tcsetattr(&original);
 * @endcode
 */
// void my_cfmakeraw(struct termios* tos)
// {
//    tos->c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP
//                            |INLCR|IGNCR|ICRNL|IXON);

//    tos->c_oflag &= ~OPOST;

//    tos->c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);

//    tos->c_cflag &= ~(CSIZE|PARENB);

//    tos->c_cflag |= CS8;
// }

void reset_screen(void)
{
   char buff[] = "Hey, baby!";
   write_to_screen("\x1b[2J\x1b[1;1H");
   write(STDOUT_FILENO, buff, strlen(buff));
}


void move_cursor(int row, int col)
{
   char buff[24];
   int mlen = snprintf(buff, sizeof(buff), "\x1b[%d;%dH", row, col);
   write(STDOUT_FILENO, buff, mlen);
}

void get_screen_size(int *row, int *col)
{
   (*row) = 0;
   (*col) = 0;
   struct winsize ws;
   int result = ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
   if (result == 0)
   {
      *row = ws.ws_row;
      *col = ws.ws_col;
   }
}

void set_scroll_limits(int top, int bottom)
{
   char buff[24];
   int mlen = snprintf(buff, sizeof(buff), "\x1b[%d;%dr", top, bottom);
   write(STDOUT_FILENO, buff, mlen);
   write_to_screen("\x1b[?6h");  // set cursor address to scroll region
}

void clear_scroll_limits(void)
{
   set_scroll_limits(1, 1000);
   write_to_screen("\x1b[?6l");  // set cursor address to window region
}

void hide_cursor(void)
{
   write_to_screen("\x1b[?25l");
}

void restore_cursor(void)
{
   write_to_screen("\x1b[?25h");
}

void select_forward_line(void)
{
   move_cursor(1,1);
   write_to_screen("\x1b[D");  // Linefeed
}

void select_reverse_line(void)
{
   move_cursor(1,1000);
   write_to_screen("\x1b[M");  // Reverse-linefeed
}

SCR_FUNCS scrf = {
   move_cursor, get_screen_size, set_scroll_limits,
   clear_scroll_limits, hide_cursor, restore_cursor,
   select_forward_line, select_reverse_line };

int pwb_printer(int row_index, int indicated, int length, void *datasource)
{
   SAS *sas = (SAS*)datasource;
   if (row_index >= 0 && row_index < sas->length)
      write_printf("%s", sas->source[row_index]);

   return 0;
}

void test_move_cursor(void)
{
   for (int row=10, col=20; row<20; ++row, col+=8)
   {
      move_cursor(row, col);
      write_printf("row: %d, col: %d", row, col);
   }
}

void test_print_page(SAS *sas)
{
   DPARMS parms = { 0 };
   initialize_dparms(&parms, sas, sas->length, pwb_printer, &scrf);

   set_screen_margins(&parms, 10, 10, -1, -1 );
   print_page(&parms);
}




int main(int argc, const char **argv)
{
   reset_screen();
   // test_move_cursor();

   SAS sas = { dsource, sizeof(dsource) / sizeof(char*) };
   test_print_page(&sas);

   return 0;
}

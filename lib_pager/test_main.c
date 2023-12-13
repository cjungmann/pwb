#include <stdio.h>
#include <string.h>       // strlen()

#include <unistd.h>       // write()

#include <stdarg.h>       // for va_list, etc
#include <alloca.h>

#include <assert.h>
#include <sys/ioctl.h>    // for ioctl() in get_screen_size()

#include "pager.h"
#include "../lib_freader/freader.h"
#include "../lib_cons/cons.h"

/**
 * @defground TEMP_TERM_FUNCS Temporary hard-coded terminal codes
 *
 * Implement a minimal set of terminal functions necessary to run the pager
 * @{
 */

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

   if (len <= 64)
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

void pager_reset_screen(void)
{
   write_to_screen("\x1b[2J\x1b[1;1H");
}

void pager_move_cursor(int row, int col)
{
   char buff[24];
   int mlen = snprintf(buff, sizeof(buff), "\x1b[%d;%dH", row, col);
   write(STDOUT_FILENO, buff, mlen);
}

void pager_get_screen_size(int *row, int *col)
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

void pager_set_scroll_limits(int top, int bottom)
{
   char buff[24];
   int mlen = snprintf(buff, sizeof(buff), "\x1b[%d;%dr", top, bottom);
   write(STDOUT_FILENO, buff, mlen);
   write_to_screen("\x1b[?6h");  // set coordinates to scroll-region area
}

void pager_clear_scroll_limits(void)
{
   pager_set_scroll_limits(1, 1000);
   write_to_screen("\x1b[?6l");  // set cursor address to window region
}

void pager_hide_cursor(void)
{
   write_to_screen("\x1b[?25l");
}

void pager_restore_cursor(void)
{
   write_to_screen("\x1b[?25h");
}

void select_forward_line(void)
{
   write_to_screen("\x0A");
   // write_to_screen("\x1b[D");  // Linefeed, text moves up
}

void select_reverse_line(void)
{
   write_to_screen("\x1bM");  // Reverse-linefeed, text moves down
}

/**
 * end of TEMP_TERM_FUNCS group
 * @}
 */



SCR_FUNCS scrf = {
   pager_move_cursor, pager_get_screen_size, pager_set_scroll_limits,
   pager_clear_scroll_limits, pager_hide_cursor, pager_restore_cursor,
   select_forward_line, select_reverse_line };

int index_printer(int row_index, int indicated, int length, void *datasource)
{
   LINDEX *index = (LINDEX*)datasource;

   if (row_index >= 0 && row_index < index->count)
   {
      if (indicated)
         write_to_screen("\x1b[7m");

      write_printf(get_lindex_line(index, row_index));

      if (indicated)
         write_to_screen("\x1b[27m");
   }

   return 0;
}

typedef struct termcap_entry_action {
   TCENTRY entry;
   PACTION action;
} TCE_ACTION;

TCE_ACTION keys[] = {
   { {"kcud1"}, pager_focus_down_one },    // key down
   { {"kcuu1"}, pager_focus_up_one },    // key up
   { {"knp"},  pager_focus_down_page },    // key next page (page down)
   { {"kpp"}, pager_focus_up_page  },    // key previous page (page up)
   { {NULL}, NULL }        // terminator entry
};

int handle_keystroke(const char *kstk, TCE_ACTION *ta_keys, DPARMS *parms)
{
   TCE_ACTION *ptr = ta_keys;
   while (ptr->entry.name)
   {
      if (strcmp(ptr->entry.value, kstk)==0)
      {
         (*ptr->action)(parms);
         return 1;
      }
      ++ptr;
   }
   return 0;
}

void test_pager_move_cursor(void)
{
   for (int row=10, col=20; row<20; ++row, col+=8)
   {
      pager_move_cursor(row, col);
      write_printf("row: %d, col: %d", row, col);
   }
}

/**
 * @brief Test behavior of scroll region
 *
 * For reference:
 * - pager_reset_screen
 * - pager_get_screen_size
 * - pager_set_scroll_limits
 * - pager_move_cursor
 * - select_forward_line
 * - select_reverse_line
 *
 * - write_to_screen
 * - write_printf
 */
void test_scroll_window_addressing(void)
{
   int rows_size, cols_size;
   pager_get_screen_size(&rows_size, &cols_size);

   int marg_top = 0, marg_bottom = 0;
   int window_size = 3;
   int total_vertical_margins = rows_size - window_size;
   if (total_vertical_margins % 2)
   {
      marg_top = (total_vertical_margins + 1) / 2;
      marg_bottom = marg_top - 1;
   }
   else
      marg_top = marg_bottom = total_vertical_margins / 2;

   pager_set_scroll_limits(marg_top, marg_top + window_size);

   pager_reset_screen();

   int row;
   for (row=0; row < 4; ++row)
   {
      pager_move_cursor(row,1);
      write_printf("Coordinates %d:%d", row, 1);
   }

   get_keystroke(NULL,0);

   pager_move_cursor(row, 1);
   select_forward_line();
   pager_move_cursor(row-1, 1);
   write_printf("Coordinates %d:%d", row, 1);

   get_keystroke(NULL,0);

   pager_move_cursor(1,1);
   select_reverse_line();
   write_printf("Coordinates %d:%d", 3, 1);

   get_keystroke(NULL,0);

   pager_set_scroll_limits(1, rows_size);
}


int run_page(LINDEX *index)
{
   pager_reset_screen();

   fill_termcap_array((TCENTRY*)keys, sizeof(TCE_ACTION));

   DPARMS parms = { 0 };
   initialize_dparms(&parms, index, index->count, index_printer, &scrf);

   set_screen_margins(&parms, 4, 4, 4, 4);
   print_page(&parms);

   char keybuff[18];
   while (1)
   {
      const char *ks = get_keystroke(keybuff, sizeof(keybuff));
      if (strcmp(ks, "q")==0)
         break;
      else
         handle_keystroke(ks, keys, &parms);
   }

   pager_set_scroll_limits(1, 999);
   return 0;
}

int test_print_page(int argc, const char **argv)
{
   int rval = 1;

   if (argc < 2)
   {
      printf("Must include a file name from which to read text lines.\n");
      goto early_exit;
   }

   const char *fname = argv[1];
   FILE *fstream = fopen(fname, "r");
   if (fstream == NULL)
   {
      printf("Cannot open file '%s'\n", fname);
      goto early_exit;
   }

   LINDEX *index = index_lines(fstream);
   // Close what we no longer need:
   fclose(fstream);

   if (index)
   {
      run_page(index);
      destroy_lindex(index);
      rval = 0;
   }
   else
      printf("Mysterious failure indexing content from '%s'.\n", fname);


  early_exit:
   return rval;
}

int run_pager(const char *file)
{
   FILE *fstream = fopen(file, "r");
   if (fstream)
   {
      LINDEX *lindex = index_lines(fstream);
      if (lindex)
      {
         destroy_lindex(lindex);
      }
   }

   return 0;
}

int main(int argc, const char **argv)
{
   int rval = 0;

   // test_pager_move_cursor();
   // test_scroll_window_addressing();
   // rval = test_print_page(argc, argv);

   // Group the next two together, commented or not:
   assert(argc>1);
   rval = run_pager(argv[1]);

   return rval;
}

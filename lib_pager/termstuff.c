#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>    // for get_env()
#include <sys/ioctl.h>

#include <assert.h>
#include <alloca.h>    // needed by ti_printf

#include <curses.h>    // for tigetstr, tiparm
#include <term.h>

#include "export.h"
#include "termstuff.h"

/**
 * @brief Equivalent to `struct termcap_entry` found in `lib_cons/cons.h`
 *
 * Using an equivalent struct to enable cooperation between the two
 * submodules, but using a different name to preserve the disconnect
 * between the submodules.
 *
 * The values can be set by hand in a testing environment
 */
struct termcap_value {
   const char *name;
   const char *value;
};

typedef struct termcap_value TCVAL;
struct termcap_value term_vals[] = {
   { "clear" },              // reset screen
   { "cup"   },              // cursor address (move cursor to row;col)
   { "u7"    },              // cursor position report (get cursor position)
   { "csr"   },              // change scroll region
   { "ind"   },              // scroll forward (up)
   { "ri"    },              // scroll reverse (down)
   { "civis" },              // make cursor invisible
   { "cnorm" },              // make cursor normal (visible but not bold)
   { "smso"  },              // enter standout mode
   { "rmso"  },              // exit standout mode
   { "smcup" },              // enter CA mode
   { "rmcup" },              // exit CA mode
   { NULL }
};

TCVAL code_vals[] = {
   { "cl" },              // 'clear' reset screen
   { "cm" },              // 'cm'   cursor address (move to row;col)
   { "u7" },              // 'u7' conventional cursor position report
   { "cs" },              // 'csr' change scroll region
   { "sf" },              // 'ind' scroll forward
   { "sr" },              //  'ri' scroll reverse
   { "vi" },              // 'civis'  cursor invisible
   { "ve" },              // 'cnorm'  normal cursor
   { "so" },              // 'smso'   enter standout mode
   { "se" },              // 'rmso'   exit standout mode
   { "ti" },              // 'smcup'  enter ca mode
   { "te" },              // 'rmcup'  exit ca mode
   { NULL }
};

enum term_indexes {
   TI_CLEAR,
   TI_MOVE_CURSOR,
   TI_REPORT_CURSOR,
   TI_SCROLL_REGION,
   TI_SCROLL_FORWARD,
   TI_SCROLL_REVERSE,
   TI_HIDE_CURSOR,
   TI_SHOW_CURSOR,
   TI_ENTER_STANDOUT_MODE,
   TI_EXIT_STANDOUT_MODE,
   TI_ENTER_CA_MODE,
   TI_EXIT_CA_MODE,
   TI_INDEX_END
};

#define CLEAR_STR                 code_vals[TI_CLEAR].value
#define MOVE_CURSOR_STR           code_vals[TI_MOVE_CURSOR].value
#define REPORT_CURSOR_STR         code_vals[TI_REPORT_CURSOR].value
#define SCROLL_REGION_STR         code_vals[TI_SCROLL_REGION].value
#define SCROLL_FORWARD_STR        code_vals[TI_SCROLL_FORWARD].value
#define SCROLL_REVERSE_STR        code_vals[TI_SCROLL_REVERSE].value
#define HIDE_CURSOR_STR           code_vals[TI_HIDE_CURSOR].value
#define SHOW_CURSOR_STR           code_vals[TI_SHOW_CURSOR].value
#define ENTER_STANDOUT_MODE_STR   code_vals[TI_ENTER_STANDOUT_MODE].value
#define EXIT_STANDOUT_MODE_STR    code_vals[TI_EXIT_STANDOUT_MODE].value
#define ENTER_CA_MODE_STR         code_vals[TI_ENTER_CA_MODE].value
#define EXIT_CA_MODE_STR          code_vals[TI_EXIT_CA_MODE].value


bool get_term_values(void)
{
   TCVAL *ptr = term_vals;
   while (ptr->name)
   {
      const char *val = tigetstr(ptr->name);
      if ((*(int*)val) == -1)
      {
         printf("%s is not a capability string (misspelled?).\n", ptr->name);
         return false;
      }
      else if ((*(int*)val) == 0)
      {
         printf("%s is not supported.\n", ptr->name);
         return false;
      }
      else
         ptr->value = val;

      ++ptr;
   }

   return true;
}

const char *get_less_termcap_val(const char *code)
{
   static char ename[] = "LESS_TERMCAP_xx";
   static char *ecode = &ename[13];

   // A long name is a coding error:
   assert(strlen(code)==2);
   // Despite assert, let's enforce two-byte copy
   memcpy(ecode, code, 2);

   return getenv(ename);
}

bool get_code_values(void)
{
   TCVAL *ptr = code_vals;
   while (ptr->name)
   {
      const char *val = get_less_termcap_val(ptr->name);
      if (val == NULL)
         val = tgetstr(ptr->name, NULL);

      if (val)
         ptr->value = val;
      else
      {
         printf("Failed to find termcap code %s.\n", ptr->name);
         exit(1);
      }

      ++ptr;
   }

   return true;
}



void ti_write_str(const char *str)
{
   int len = strlen(str);
   write(STDOUT_FILENO, str, len);
   // tputs(str, 1, putchar);
}

int ti_printf(const char *fmt, ...)
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

   // tputs(buff, 1, putchar);
   write(STDOUT_FILENO, buff, len);

   va_end(args);

   return len;
}


EXPORT void ti_start_term(void)
{
   // Let setupterm notify and exit on error:
   // setupterm(NULL, STDOUT_FILENO, (int*)0);
   // get_term_values();
   get_code_values();
   ti_write_str(ENTER_CA_MODE_STR);
   ti_reset_screen();
}

EXPORT void ti_cleanup_term(void)
{
   ti_write_str(EXIT_CA_MODE_STR);
}

EXPORT void ti_reset_screen(void)
{
   ti_write_str(CLEAR_STR);
}

EXPORT void ti_start_standout(void)
{
   ti_write_str(ENTER_STANDOUT_MODE_STR);
}

EXPORT void ti_end_standout(void)
{
   ti_write_str(EXIT_STANDOUT_MODE_STR);
}

void ti_set_cursor_position(int row, int col)
{
   const char *str = tiparm(MOVE_CURSOR_STR, row, col);
   ti_write_str(str);
}

void ti_get_cursor_position(int *row, int *col)
{
   int fh = STDOUT_FILENO;
   ti_write_str(REPORT_CURSOR_STR);

   struct termios original, raw;
   tcgetattr(fh, &original);
   raw = original;
   raw.c_lflag &= ~(ECHO|ECHONL|ICANON|ISIG|IEXTEN);
   tcsetattr(fh, TCSANOW, &raw);

   scanf("\x1b[%d;%dR", row, col);

   tcsetattr(fh, TCSANOW, &original);
}

void ti_get_screen_size(int *rows, int *cols)
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
      ti_get_cursor_position(&strow, &stcol);
      ti_set_cursor_position(999,999);
      ti_get_cursor_position(rows,cols);
      ti_set_cursor_position(strow, stcol);
   }
}

void ti_set_scroll_limit(int top, int count)
{
   const char *str = tiparm(SCROLL_REGION_STR, top, top + count);
   ti_write_str(str);
}

/**
 * @brief AKA scroll DOWN
 */
void ti_scroll_forward(void)
{
   ti_write_str(SCROLL_FORWARD_STR);
}

/**
 * @brief ATA scroll UP
 */
void ti_scroll_reverse(void)
{
   ti_write_str(SCROLL_REVERSE_STR);
}




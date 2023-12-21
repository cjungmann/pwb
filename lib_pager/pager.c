#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include <curses.h>  // for tigetstr, tiparm
#include <term.h>

#include "export.h"
#include "termstuff.h"
#include "pager.h"


EXPORT void print_page(DPARMS *params)
{
   int left = params->chars_left;
   int line = params->line_top;
   int line_limit = line + params->line_count;
   int chars_count = params->chars_count;

   int row = params->index_row_top;
   int end_row = row + params->line_count;
   if (end_row >= params->row_count)
      end_row = params->row_count-1;

   for (; line < line_limit; ++row, ++line)
   {
      ti_set_cursor_position(line, left);

      // Erase the line before requesting a reprint.
      // This is probably not necessary, plan to delete it.
      ti_printf("\x1b[%dX", chars_count);

      if (row <= end_row)
      {
         bool has_focus = row == params->index_row_focus;
         (params->printer)(row, has_focus, chars_count, params->data_source);
      }
      else
         (params->printer)(row, false, chars_count, params->data_source);
   }
}


EXPORT int pager_begin(DPARMS *parms, KEYMAP *keymap, KEYSTROKE_GETTER ksg)
{
   print_page(parms);

   char keybuff[24];
   ARV arv = ARV_CONTINUE;

   while(arv != ARV_EXIT)
   {
      const char *ks = (*ksg)(keybuff, sizeof(keybuff));
      if (ks)
      {
         arv = (*keymap->kc_func)(keymap, parms, ks);
         if (arv == ARV_REPLOT_DATA)
            print_page(parms);
      }
   }

   return 0;
}

#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "export.h"
#include "pager.h"
#include "screen.h"
#include "get_keystroke.h"

EXPORT PKMAP sel_keymap[] = {
   { PAGER_EXIT,      "q" },         // 'q' key to quit
   { PAGER_SELECT,    "\x0D" },      // NEWLINE (ENTER key) to select (\x0D CR, \x0A for NL)
   { FOCUS_DOWN_ONE,  "kcud1" },     // down key
   { FOCUS_UP_ONE,    "kcuu1" },     // up key
   { FOCUS_HOME,      "khome" },     // home key
   { FOCUS_END,       "kend" },      // end key
   { FOCUS_DOWN_PAGE, "knp" },       // next-page key (pgdn)
   { FOCUS_UP_PAGE,   "kpp" },       // previous-page key (pgup)
   { -1 }
};

EXPORT PACTION const sel_actionmap[] = {
   pager_quit,
   pager_activate,
   pager_focus_down_one,
   pager_focus_up_one,
   pager_focus_down_page,
   pager_focus_up_page,
   pager_focus_end,
   pager_focus_home,
   pager_scroll_down_one,
   pager_scroll_up_one,
   pager_scroll_down_page,
   pager_scroll_up_page,
   pager_scroll_end,
   pager_scroll_home,
   NULL
};



int get_index_bottom_limit(const DPARMS *parms)
{
   assert(parms);
   return parms->index_row_top + parms->line_count;
}

int get_index_bottom_line(const DPARMS *parms)
{
   assert(parms);
   int index = parms->index_row_top + parms->line_count - 1;
   if (index > parms->row_count - 1)
      index = parms->row_count - 1;

   return index;
}

bool row_index_is_visible(const DPARMS *parms, int row_index)
{
   int bottom_row= parms->index_row_top + parms->line_count;
   return row_index >= parms->index_row_top && row_index < bottom_row;
}

int get_line_index_from_row_index(const DPARMS *parms, int row_index)
{
   return row_index - parms->index_row_top;
}

void print_indexed_row(const DPARMS *parms, int row_index, bool has_focus)
{
   // static int prow, pcol;
   // get_screen_size(&prow, &pcol);
   int line = parms->line_top + (row_index - parms->index_row_top);
   set_cursor_position(line, parms->chars_left);
   (parms->printer)(row_index, has_focus, parms->chars_count, parms->data_source);
}

PACTION page_get_action(const DPARMS *parms, const char *keystroke)
{
   int ndx = -1;
   const PKMAP *ptr = parms->keymap;
   while (ptr->action_index >= 0)
   {
      if (0==strcmp(keystroke, ptr->value))
      {
         ndx = ptr->action_index;
         break;
      }
      ++ptr;
   }
   return parms->actions[ndx];
}

void scroll_line_up(const DPARMS *parms)
{
   assert(tis_values_set());
   int col = parms->chars_left + parms->chars_count ;
   int row = parms->line_top + parms->line_count - 1;
   set_cursor_position(row, col);
   screen_write_str(tis_scroll_forward, STDOUT_FILENO);
}

void scroll_line_down(const DPARMS *parms)
{
   assert(tis_values_set());
   set_cursor_position(parms->line_top, 1);
   screen_write_str(tis_scroll_reverse, STDOUT_FILENO);
}

void print_page(DPARMS *params)
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
      set_cursor_position(line, left);
      if (row <= end_row)
      {
         bool has_focus = row == params->index_row_focus;
         (params->printer)(row, has_focus, chars_count, params->data_source);
      }
      else
         (params->printer)(row, false, chars_count, params->data_source);
   }
}



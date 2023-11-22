#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <curses.h>
#include <term.h>
#include <assert.h>

#include "pager.h"
#include "screen.h"
#include "get_keystroke.h"

PKMAP sel_keymap[] = {
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

PACTION const sel_actionmap[] = {
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



void update_keymap_values(PKMAP *kmap, int el_size)
{
   char *ptr = (char*)kmap;
   PKMAP *pkm;
   while ((pkm=(PKMAP*)ptr)->action_index >= 0)
   {
      const char *name = pkm->capname;
      if (strlen(name) > 1 && name[0] == 'k')
      {
         char *val = tigetstr(pkm->capname);
         if (val)
            pkm->value = val;
      }
      else
         pkm->value = name;

      ptr += el_size;
   }
}

void update_display_params(DPARMS *params)
{
   int rows, cols;
   get_screen_size(&rows, &cols);

   params->line_top = params->reserve_top;
   params->line_count = rows - params->reserve_top - params->reserve_bottom;
   params->chars_left = params->reserve_left;
   params->chars_count = cols - params->reserve_left - params->reserve_right;

   set_scroll_limits(params->line_top, rows - params->reserve_bottom - 1);
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
   update_display_params(params);

   int left = params->chars_left;
   int line = params->line_top;
   int chars_count = params->chars_count;

   int row = params->index_row_top;
   int end_row = row + params->line_count;
   if (end_row >= params->row_count)
      end_row = params->row_count-1;

   for ( ; row < end_row; ++row, ++line)
   {
      bool has_focus = row == params->index_row_focus;
      set_cursor_position(line, left);
      (params->printer)(row, has_focus, chars_count, params->data_source);
   }
}


void start_pager(DPARMS *params)
{
   reset_screen();
   print_page(params);

   char kbuff[20];
   ARV action_return = ARV_CONTINUE;

   while (action_return != ARV_EXIT)
   {
      char *keys = get_keystroke(kbuff, sizeof(kbuff));
      if (keys)
      {
         PACTION action = page_get_action(params, keys);
         if (action)
            action_return = (*action)(params);
         else
            action_return = ARV_CONTINUE;
      }
   }
}

bool row_index_is_visible(const DPARMS *parms, int row_index)
{
   int bottom_row= parms->index_row_top + parms->line_count;
   return row_index >= parms->index_row_top && row_index < bottom_row;
}

int line_index_from_row_index(const DPARMS *parms, int row_index)
{
   return row_index - parms->index_row_top;
}


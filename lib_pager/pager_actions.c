#include <assert.h>
#include <stdio.h>

#include "pager.h"
#include "termstuff.h"

/**
 * @defgroup MOVEMENT_SUPPORT These functions support pager_focus_xxx functions
 * @{
 */
int get_index_bottom_line(const DPARMS *parms)
{
   assert(parms);
   // int index = parms->index_row_top + parms->line_count - 1;
   int index = parms->line_count - 1;

   if (index > parms->row_count - 1)
      index = parms->row_count - 1;

   return index;
}

bool row_index_is_visible(const DPARMS *parms, int row_index)
{
   int bottom_row = parms->index_row_top + parms->line_count;
   // return row_index >= parms->index_row_top && row_index < bottom_row;
   return row_index >= parms->index_row_top && row_index < bottom_row;
}

int get_line_index_from_row_index(const DPARMS *parms, int row_index)
{
   return row_index - parms->index_row_top + parms->line_top;
}

void print_indexed_row(const DPARMS *parms, int row_index, bool has_focus)
{
   int line = get_line_index_from_row_index(parms, row_index);
   ti_set_cursor_position(line, parms->chars_left);
   (*parms->printer)(row_index, has_focus, parms->chars_count, parms->data_source);
}

/** @} */




ARV pager_quit(DPARMS *parms)
{
   return ARV_EXIT;
}

ARV pager_activate(DPARMS *parms)
{
   return ARV_REPLOT_DATA;
}

ARV pager_focus_up_one(DPARMS *parms)
{
   // We shouldn't have to check if the focus should be on a valid row:
   assert(parms->index_row_focus < parms->row_count);

   if (parms->index_row_focus > 0)
   {
      print_indexed_row(parms, parms->index_row_focus, 0);

      --parms->index_row_focus;
      if (parms->index_row_focus < parms->index_row_top)
      {
         ti_scroll_reverse();
         --parms->index_row_top;
      }

      print_indexed_row(parms, parms->index_row_focus, 1);
   }

   return ARV_CONTINUE;
}

ARV pager_focus_down_one(DPARMS *parms)
{
   // We shouldn't have to check if the focus should be on a valid row:
   assert(parms->index_row_focus >= 0);

   int table_last_index = parms->row_count - 1;

   // If there remains table rows below the focus
   if (parms->index_row_focus < table_last_index)
   {
      // Change registered focus after unindicating current focus
      print_indexed_row(parms, parms->index_row_focus, 0);
      ++parms->index_row_focus;

      int screen_last_index = get_index_bottom_line(parms);
      if (parms->index_row_focus > screen_last_index + 1)
      {
         int row = parms->line_top + parms->line_count;
         int col = parms->chars_left + parms->chars_count;
         ti_set_cursor_position(row, col);
         ti_scroll_forward();
         ++parms->index_row_top;
      }

      print_indexed_row(parms, parms->index_row_focus, 1);
   }

   return ARV_CONTINUE;
}

ARV pager_focus_down_page(DPARMS *parms)
{
   int index_bottom_line = get_index_bottom_line(parms);
   int at_line = get_line_index_from_row_index(parms, index_bottom_line);

   // If last row of content is on screen, set focus on it
   if (at_line < parms->line_count-1)
   {
      if (parms->index_row_focus < parms->row_count-1)
      {
         print_indexed_row(parms, parms->index_row_focus, 0);
         parms->index_row_focus = index_bottom_line;
         print_indexed_row(parms, parms->index_row_focus, 1);
      }
   }
   else
   {
      parms->index_row_top = parms->index_row_focus = index_bottom_line;
      return ARV_REPLOT_DATA;
   }

   return ARV_CONTINUE;
}

ARV pager_focus_up_page(DPARMS *parms)
{
   int index_top_line = parms->index_row_top;
   if (index_top_line == 0)
   {
      if (parms->index_row_focus > index_top_line)
      {
         print_indexed_row(parms, parms->index_row_focus, 0);
         parms->index_row_focus = index_top_line;
         print_indexed_row(parms, parms->index_row_focus, 1);
      }
   }
   else
   {
      index_top_line -= parms->line_count;
      if (index_top_line < 0)
         index_top_line = 0;

      parms->index_row_top = parms->index_row_focus = index_top_line;
      return ARV_REPLOT_DATA;
   }
   return ARV_CONTINUE;
}

ARV pager_focus_end(DPARMS *parms)
{
   int index_last_row = parms->row_count - 1;
   if (row_index_is_visible(parms, index_last_row))
   {
      print_indexed_row(parms, parms->index_row_focus, 0);
      parms->index_row_focus = index_last_row;
      print_indexed_row(parms, parms->index_row_focus, 1);
   }
   else
   {
      parms->index_row_top = index_last_row - parms->line_count + 1;
      if (parms->index_row_top < 0)
         parms->index_row_top = 0;
      parms->index_row_focus = index_last_row;
      return ARV_REPLOT_DATA;
   }
   return ARV_CONTINUE;
}

ARV pager_focus_home(DPARMS *parms)
{
   if (row_index_is_visible(parms, 0))
   {
      print_indexed_row(parms, parms->index_row_focus, 0);
      parms->index_row_focus = 0;
      print_indexed_row(parms, parms->index_row_focus, 1);
   }
   else
   {
      parms->index_row_top = parms->index_row_focus = 0;
      return ARV_REPLOT_DATA;
   }
   return ARV_CONTINUE;
}

ARV pager_scroll_down_one(DPARMS *parms)
{
   return ARV_CONTINUE;
}
ARV pager_scroll_up_one(DPARMS *parms)
{
   return ARV_CONTINUE;
}
ARV pager_scroll_down_page(DPARMS *parms)
{
   return ARV_CONTINUE;
}
ARV pager_scroll_up_page(DPARMS *parms)
{
   return ARV_CONTINUE;
}
ARV pager_scroll_end(DPARMS *parms)
{
   return ARV_CONTINUE;
}
ARV pager_scroll_home(DPARMS *parms)
{
   return ARV_CONTINUE;
}


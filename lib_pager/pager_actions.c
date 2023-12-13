#include <assert.h>
#include <stdio.h>

#include "pager.h"

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
         scroll_line_down(parms);
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
      if (parms->index_row_focus > screen_last_index)
      {
         scroll_line_up(parms);
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


#include <assert.h>
#include <stdio.h>

#include "pager.h"
#include "screen.h"

ARV pager_quit(DPARMS *parms)
{
   printf("QUITTING, baby!\n");
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
      print_indexed_row(parms, parms->index_row_focus, 0);
      ++parms->index_row_focus;

      int screen_last_index = parms->index_row_top + parms->line_count - 1;
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
   int old_focus = parms->index_row_focus;

   int index_bottom_limit = parms->index_row_top + parms->line_count;
   if (old_focus < index_bottom_limit)
   {
   }
   return ARV_CONTINUE;
}
ARV pager_focus_up_page(DPARMS *parms)
{
   return ARV_CONTINUE;
}
ARV pager_focus_end(DPARMS *parms)
{
   return ARV_CONTINUE;
}
ARV pager_focus_home(DPARMS *parms)
{
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


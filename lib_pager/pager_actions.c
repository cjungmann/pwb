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
   return parms->index_row_top + parms->line_count - 1;
}

/**
 * @brief Test for row visibility to help decide if replot is necessary
 * @param "parms"     Active pager control data
 * @param "row_index" location in data source for which the test is run.
 * @return `true` if the row is visible, `false` if not.
 */
bool row_index_is_visible(const DPARMS *parms, int row_index)
{
   int bottom_row = get_index_bottom_line(parms);
   return row_index >= parms->index_row_top && row_index <= bottom_row;
}

/**
 * @brief Alias for calculation for clarity of intention
 */
int get_line_index_from_row_index(const DPARMS *parms, int row_index)
{
   return parms->line_top + row_index - parms->index_row_top;
}

/**
 * @brief Internal function to print a row using print function pointer in `parms'.
 * @param "parms"     Active pager control data
 * @param "row_index" index in data source for row to be printed
 * @param "has_focus" flag to trigger printing line in standout mode
 */
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
      if (parms->index_row_focus > screen_last_index)
      {
         int row = parms->line_top + parms->line_count - 1;
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
   // int index_bottom_line = get_index_bottom_line(parms);
   // int at_line = get_line_index_from_row_index(parms, index_bottom_line);

   int new_focus_index = parms->index_row_focus + 10;
   if (new_focus_index >= parms->row_count)
      new_focus_index = parms->row_count - 1;

   // Proceed only if the new_row_index differs from the current focus index
   if (new_focus_index != parms->index_row_focus)
   {
      // unfocus currently focused line
      print_indexed_row(parms, parms->index_row_focus, 0);

      // change focused line for whatever follows
      parms->index_row_focus = new_focus_index;

      if (row_index_is_visible(parms, new_focus_index))
         print_indexed_row(parms, parms->index_row_focus, 1);
      else
      {
         int new_top_row = new_focus_index - parms->line_count + 1;
         if (new_top_row < 0)
            new_top_row = 0;
         parms->index_row_top = new_top_row;
         return ARV_REPLOT_DATA;
      }
   }

   return ARV_CONTINUE;
}

ARV pager_focus_up_page(DPARMS *parms)
{
   // If focus already on top line, we'll pan or abort
   if (parms->index_row_focus == parms->index_row_top)
   {
      // There's nothing to do if top_line is row 0 and has focus;
      if (parms->index_row_top != 0)
      {
         // Move back a pageful
         int new_top_row = parms->index_row_top - parms->line_count;
         if (new_top_row < 0)
            new_top_row = 0;

         parms->index_row_focus = parms->index_row_top = new_top_row;
         return ARV_REPLOT_DATA;
      }
   }
   else // We're staying with the current set of lines
   {
      // Unfocus line (since we'll still be seeing it)
      print_indexed_row(parms, parms->index_row_focus, 0);
      parms->index_row_focus = parms->index_row_top;
      print_indexed_row(parms, parms->index_row_focus, 1);
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


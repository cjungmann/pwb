#include <string.h>
#include <curses.h>
#include <term.h>

#include "export.h"
#include "termstuff.h"
#include "pager.h"


/**
 * @brief Sets up a DPARMS variable with minimum necessary values.
 * @param "parms"       struct to be initialized
 * @param "data_source" pointer to datasource to be used for content
 * @param "row_count"   number of records/rows in data source
 * @param "printer"     function to be used for printing lines
 */
EXPORT void pager_init_dparms(DPARMS *parms,
                              void *data_source,
                              int row_count,
                              pwb_print_line printer)
{
   memset(parms, 0, sizeof(DPARMS));
   parms->data_source = data_source;
   parms->row_count = row_count;
   parms->printer = printer;

   parms->index_row_top = 0;
   parms->index_row_focus = 0;

   pager_set_margins(parms, 0, 0, 0, 0);
}

/**
 * @brief Set output dimensions based on margin sizes
 * @param "parms"    Parameter struct in which dimensions are to be set
 * @param "top"      top margin in lines
 * @param "right"    right margin in characters
 * @param "bottom"   bottom margin in lines
 * @param "left"     left margin in lines
 *
 * Set a negative value to use a calculated margin value.
 * Following the CSS margins convention:
 * - only @p top is set, its value applies to all four margins
 * - only @p top and @p right set, @p top applies to @top and @bottom,
 *   and @p right applies to @p right and @p left.
 * - only@p top, @p right, and @p bottom set, @p right applies to @p right and @p left.
 */
EXPORT void pager_set_margins(DPARMS *parms, int top, int right, int bottom, int left)
{
   // Adjust to missing values
   if (top < 0)
      top = right = bottom = left = 0;
   else if (right < 0)
      right = bottom = left = top;
   else if (bottom < 0)
   {
      bottom = top;
      left = right;
   }
   else if (left < 0)
      left = right;

   int rows, cols;
   ti_get_screen_size(&rows, &cols);

   parms->line_top = top;
   parms->line_count = rows - top - bottom;
   parms->chars_left = left;
   parms->chars_count = cols - left - right;

   ti_set_scroll_limit(top, parms->line_count);
}


#include <string.h>

#include "pager.h"
#include "screen.h"
#include "get_keystroke.h"


void initialize_dparms(DPARMS *parms, void *data_source, int row_count, pwb_print_line printer)
{
   memset(parms, 0, sizeof(DPARMS));
   parms->data_source = data_source;
   parms->row_count = row_count;
   parms->printer = printer;

   parms->index_row_top = 0;
   parms->index_row_focus = 0;

   parms->actions = sel_actionmap;
   parms->keymap = sel_keymap;

   set_screen_margins(parms, 0, 0, 0, 0);
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
void set_screen_margins(DPARMS *parms, int top, int right, int bottom, int left)
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
   get_screen_size(&rows, &cols);

   parms->line_top = top;
   parms->line_count = rows - top - bottom;
   parms->chars_left = left;
   parms->chars_count = cols - left - right;

   set_scroll_limits(top, rows - bottom - 1);
}

/**
 * @brief Placeholder for future mechanism for keymap manipulation
 */
void replace_keymap(DPARMS *parms, PKMAP *new_map)
{
   ;
}


void start_pager(DPARMS *params)
{
   reset_screen();
   pwb_enter_ca_mode();

   char kbuff[20];
   ARV action_return = ARV_REPLOT_DATA;

   while (action_return != ARV_EXIT)
   {
      if (action_return == ARV_REPLOT_DATA)
         print_page(params);

      action_return = ARV_CONTINUE;

      char *keys = get_keystroke(kbuff, sizeof(kbuff));
      if (keys)
      {
         PACTION action = page_get_action(params, keys);
         if (action)
            action_return = (*action)(params);
      }
   }

   clear_scroll_limits();
   pwb_exit_ca_mode();
}


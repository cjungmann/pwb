#include <string.h>
#include <curses.h>
#include <term.h>

#include "export.h"
#include "pager.h"


// EXPORT void update_keymap_values(PKMAP *kmap, int el_size)
// {
//    char *ptr = (char*)kmap;
//    PKMAP *pkm;
//    while ((pkm=(PKMAP*)ptr)->action_index >= 0)
//    {
//       const char *name = pkm->capname;
//       if (strlen(name) > 1 && name[0] == 'k')
//       {
//          char *val = tigetstr(pkm->capname);
//          if (val)
//             pkm->value = val;
//       }
//       else
//          pkm->value = name;

//       ptr += el_size;
//    }
// }

EXPORT void initialize_dparms(DPARMS *parms,
                              void *data_source,
                              int row_count,
                              pwb_print_line printer,
                              SCR_FUNCS *funcs
   )
{
   memset(parms, 0, sizeof(DPARMS));
   parms->data_source = data_source;
   parms->row_count = row_count;
   parms->printer = printer;
   parms->funcs = funcs;

   parms->index_row_top = 0;
   parms->index_row_focus = 0;

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
EXPORT void set_screen_margins(DPARMS *parms, int top, int right, int bottom, int left)
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
   PARAM_GET_SCREEN_SIZE(parms, &rows, &cols);

   parms->line_top = top;
   parms->line_count = rows - top - bottom;
   parms->chars_left = left;
   parms->chars_count = cols - left - right;

   PARAM_SET_SCROLL_LIMIT(parms, top, top + parms->line_count);
}


// EXPORT void start_pager(DPARMS *params)
// {
//    reset_screen();
//    pwb_enter_ca_mode();

//    char kbuff[20];
//    ARV action_return = ARV_REPLOT_DATA;

//    while (action_return != ARV_EXIT)
//    {
//       if (action_return == ARV_REPLOT_DATA)
//          print_page(params);

//       action_return = ARV_CONTINUE;

//       char *keys = get_keystroke(kbuff, sizeof(kbuff));
//       if (keys)
//       {
//          PACTION action = page_get_action(params, keys);
//          if (action)
//             action_return = (*action)(params);
//       }
//    }

//    clear_scroll_limits();
//    pwb_exit_ca_mode();
// }


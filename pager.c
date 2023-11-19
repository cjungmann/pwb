#include <stdbool.h>

#include "pager.h"
#include "screen.h"

void update_display_params(DPARMS *params)
{
   int rows, cols;
   get_screen_size(&rows, &cols);

   params->line_top = params->reserve_top;
   params->line_count = rows - params->reserve_top - params->reserve_bottom;
   params->chars_left = params->reserve_left;
   params->chars_count = cols - params->reserve_left - params->reserve_right;
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
      bool selected = row == params->index_row_selected;
      set_cursor_position(line, left);
      (params->printer)(row, selected, chars_count, params->data_source);
   }
}


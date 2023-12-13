#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

#include "export.h"
#include "pager.h"


int get_index_bottom_limit(const DPARMS *parms)
{
   assert(parms);
   return parms->index_row_top + parms->line_count;
}

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
   return row_index - parms->index_row_top;
}

void print_indexed_row(const DPARMS *parms, int row_index, bool has_focus)
{
   // static int prow, pcol;
   // get_screen_size(&prow, &pcol);
   int line = row_index - parms->index_row_top + 1;
   PARAM_MOVE(parms, line, parms->chars_left);
   (parms->printer)(row_index, has_focus, parms->chars_count, parms->data_source);
}

void scroll_line_up(const DPARMS *parms)
{
   int col = parms->chars_left + parms->chars_count;
   PARAM_MOVE(parms, parms->line_count+1, col);
   PARAM_SCROLL_FORWARD(parms);
}

void scroll_line_down(const DPARMS *parms)
{
   PARAM_MOVE(parms, 1, 1);
   PARAM_SCROLL_REVERSE(parms);
}

EXPORT void print_page(DPARMS *params)
{
   int left = params->chars_left;
   int line = 1;
   int line_limit = line + params->line_count;
   int chars_count = params->chars_count;

   int row = params->index_row_top;
   int end_row = row + params->line_count;
   if (end_row >= params->row_count)
      end_row = params->row_count-1;

   for (; line < line_limit; ++row, ++line)
   {
      PARAM_MOVE(params, line, left);
      if (row <= end_row)
      {
         bool has_focus = row == params->index_row_focus;
         (params->printer)(row, has_focus, chars_count, params->data_source);
      }
      else
         (params->printer)(row, false, chars_count, params->data_source);
   }
}



#ifndef PAGER_H
#define PAGER_H

typedef struct display_params DPARMS;

typedef int (*pwb_print_line)(int row_index, int indicated, int length, void *dsource);

struct display_params {
   int reserve_top;         ///< lines at top left alone
   int reserve_right;       ///< chars to right left alone
   int reserve_bottom;      ///< lines below left alone
   int reserve_left;        ///< chars to left left alone

   int index_row_top;       ///< index number of top row of source
   int index_row_selected;  ///< index of row that is currently 'selected'
   int row_count;           ///< number of rows in the source

   void *data_source;       ///< passed to print function for printing lines
   pwb_print_line printer;  ///< function pointer to be called for each output line

   int line_top;            ///< screen line to begin printing
   int line_count;          ///< number of screen lines in region
   int chars_left;          ///< left margin
   int chars_count;         ///< number of characters to print per linecsscss
};

void update_display_params(DPARMS *params);
void print_page(DPARMS *params);
#endif

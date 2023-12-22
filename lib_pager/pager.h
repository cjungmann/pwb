#ifndef PAGER_H
#define PAGER_H

#include <stdbool.h>

/**
 * @brief Return value from pager actions, indicating action to take
 */
typedef enum action_return_values {
   ARV_CONTINUE = 0,
   ARV_REPLOT_DATA,
   ARV_EXIT
} ARV;

typedef struct display_params DPARMS;
typedef struct keymap KEYMAP;

typedef int (*pwb_print_line)(int row_index, int indicated, int length, void *data_source);
typedef char* (*KEYSTROKE_GETTER)(char *buff, int buff_len);
typedef ARV (*KEYCALL)(DPARMS *parms, KEYMAP *km, const char *keystroke);
typedef ARV (*PACTION)(DPARMS*);


/**
 * @brief "base class" for keymap implementations
 */
struct keymap {
   KEYCALL kc_func;    ///< function pointer to be called for any keystroke
};

/**
 * @brief Parameters needed to run the pager.
 *
 * The first two members, @p index_row_top and @p index_row_focus,
 * track the content of the page.
 *
 * The next two members, @p data_source and @p row_count, abstract
 * the data so it can be passed to the printer function at appropriate
 * times.
 *
 * The final required member is @p printer, which is called by the
 * pager to render every line.
 *
 * The `margin` members should be initialized to `0` to start, and
 * modified through the @ref set_screen_margins function, which will
 * calculate the writing area, and thereafter set the final members
 * in consultation of the screen dimensions.
 */
struct display_params {
   // Default top and focus rows is 0 (first record of data source):
   int index_row_top;       ///< index number of top row of source
   int index_row_focus;     ///< index of row that currently has focus'

   // Next four members are required parameters:
   void *data_source;       ///< passed to print function for printing lines
   int row_count;           ///< number of rows in the source
   pwb_print_line printer;  ///< function pointer to be called for each output line

   // Default values of 0, use function set_screen_margins() to change
   int margin_top;          ///< lines at top left alone
   int margin_right;        ///< chars to right left alone
   int margin_bottom;       ///< lines below left alone
   int margin_left;         ///< chars to left left alone

   int line_top;            ///< screen line to begin printing
   int line_bottom;         ///< screen line of bottom row of scroll window
   int line_count;          ///< number of screen lines in region
   int chars_left;          ///< left margin
   int chars_count;         ///< number of characters to print per line
};


/**
 * @defgroup LIB_PUBLIC Public library functions
 * @brief Functions found in `pager_params.c`
 * @{
 */
void pager_init_dparms(DPARMS *parms,
                       void *data_source,
                       int row_count,
                       pwb_print_line printer);

void pager_set_margins(DPARMS *parms, int top, int right, int bottom, int left);

/** @} */

void print_page(DPARMS *params);


// void start_pager(DPARMS *parms);

/** @} */

int pager_begin(DPARMS *parms, KEYMAP *keymap, KEYSTROKE_GETTER get_keystroke);

/**
 * @defgroup PAGER_MANAGMENT Functions to be used to run the page
 * These are the builtin pager management functions.  They are meant to
 * be called in response to user input.
 * @{
 */


ARV pager_quit(DPARMS *parms);
ARV pager_activate(DPARMS *parms);

ARV pager_focus_up_one(DPARMS *parms);
ARV pager_focus_down_one(DPARMS *parms);
ARV pager_focus_down_page(DPARMS *parms);
ARV pager_focus_up_page(DPARMS *parms);
ARV pager_focus_end(DPARMS *parms);
ARV pager_focus_home(DPARMS *parms);

ARV pager_scroll_down_one(DPARMS *parms);
ARV pager_scroll_up_one(DPARMS *parms);
ARV pager_scroll_down_page(DPARMS *parms);
ARV pager_scroll_up_page(DPARMS *parms);
ARV pager_scroll_end(DPARMS *parms);
ARV pager_scroll_home(DPARMS *parms);

/**
 * end of PAGER_MANAGEMENT group
 * @}
 */

#endif

#ifndef PAGER_H
#define PAGER_H

#include <stdbool.h>

typedef struct display_params DPARMS;
typedef struct pager_keymap PKMAP;

typedef enum action_return_values {
   ARV_CONTINUE = 0,
   ARV_REPLOT_DATA,
   ARV_EXIT
} ARV;

typedef ARV (*PACTION)(DPARMS*);

typedef int (*pwb_print_line)(int row_index, int indicated, int length, void *data_source);

// typedef struct pager_keymap PKMAP;
struct pager_keymap {
   int action_index;         ///< index into array of function pointers
   const char *capname;      ///< termcap name for key
   const char *value;        ///< string emitted when the key is pressed
};

#include "pager_internal.h"

/**
 * @defgroup LIB_PUBLIC Public library functions
 * @brief Functions found in `pager_public.c`
 * @{
 */
void initialize_dparms(DPARMS *parms, void *data_source, int row_count, pwb_print_line printer);
void set_screen_margins(DPARMS *parms, int top, int right, int bottom, int left);
void replace_keymmap(DPARMS *parms, PKMAP *new_map);

void start_pager(DPARMS *parms);


/** @} */


struct display_params {
   // First three members are required parameters:
   void *data_source;       ///< passed to print function for printing lines
   int row_count;           ///< number of rows in the source
   pwb_print_line printer;  ///< function pointer to be called for each output line

   // Default top and focus rows is 0 (first record of data source):
   int index_row_top;       ///< index number of top row of source
   int index_row_focus;     ///< index of row that currently has focus'

   // Default arrays are `sel_keymap` and `sel_actionmap`
   const PACTION *actions;  ///< function table of actions to be performed on matching keystroke
   const PKMAP *keymap;     ///< 'table' associating keystrokes to indexes to a function
                            ///  table of actions

   // Default values of 0, use function set_screen_margins() to change
   int margin_top;          ///< lines at top left alone
   int margin_right;        ///< chars to right left alone
   int margin_bottom;       ///< lines below left alone
   int margin_left;         ///< chars to left left alone

   int line_top;            ///< screen line to begin printing
   int line_count;          ///< number of screen lines in region
   int chars_left;          ///< left margin
   int chars_count;         ///< number of characters to print per line
};

enum drive_values {
   PAGER_EXIT = 0,
   PAGER_SELECT,
   // Movement actions:
   FOCUS_DOWN_ONE,
   FOCUS_UP_ONE,
   FOCUS_DOWN_PAGE,
   FOCUS_UP_PAGE,
   FOCUS_END,
   FOCUS_HOME,
   // Future actions
   SCROLL_DOWN_ONE,
   SCROLL_UP_ONE,
   SCROLL_DOWN_PAGE,
   SCROLL_UP_PAGE,
   SCROLL_END,
   SCROLL_HOME
};


#endif

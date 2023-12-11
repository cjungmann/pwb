#ifndef PAGER_DEFS_H
#define PAGER_DEFS_H

// Forward-declare for function pointer parameter
typedef struct display_params DPARMS;

/**
 * @brief Return value from pager actions, indicating action to take
 */
typedef enum action_return_values {
   ARV_CONTINUE = 0,
   ARV_REPLOT_DATA,
   ARV_EXIT
} ARV;
/**
 * @brief Prototype function pointer to pager action
 */
typedef ARV (*PACTION)(DPARMS*);


typedef void(*pwb_getter)(int *row, int *col);
typedef void(*pwb_two_ints)(int ione, int itwo);
typedef void(*pwb_screener)(void);
typedef struct pwb_screen_funcs {
   pwb_two_ints    mover;           ///< function to move the cursor
   pwb_getter      size_getter;     ///< function to get screen size
   pwb_two_ints    limiter;         ///< function to set scrolling limits
   pwb_screener    unlimiter;       ///< clear scrolling limits
   pwb_screener    hide_c;          ///< function to hide cursor
   pwb_screener    restore_c;       ///< function to restore cursor
   pwb_screener    forward_one;
   pwb_screener    reverse_one;
} SCR_FUNCS;

#define PARAM_GET_SCREEN_SIZE(p, rows, cols) (*(p)->funcs->size_getter)((rows),(cols))
#define PARAM_SET_SCROLL_LIMIT(p, rows, cols) (*(p)->funcs->limiter)((top),(bottom))
#define PARAM_CLEAR_SCROLL_LIMIT(p) (*(p)->funcs->unlimiter)()
#define PARAM_MOVE(p,row,col) (*(p)->funcs->mover)((row),(col))
#define PARAM_SCROLL_FORWARD(p) (*(p)->funcs->forward_one)()
#define PARAM_SCROLL_REVERSE(p) (*(p)->funcs->reverse_one)()

struct display_params {
   // Default top and focus rows is 0 (first record of data source):
   int index_row_top;       ///< index number of top row of source
   int index_row_focus;     ///< index of row that currently has focus'

   // First three members are required parameters:
   void *data_source;       ///< passed to print function for printing lines
   int row_count;           ///< number of rows in the source
   pwb_print_line printer;  ///< function pointer to be called for each output line
   SCR_FUNCS *funcs;        ///< struct with several screen-manipulation function pointers


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

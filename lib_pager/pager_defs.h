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


typedef struct keymap KEYMAP;

typedef char *(*KEYSTROKE_GETTER)(char *buff, int buff_len);
typedef ARV (*KEYCALL)(KEYMAP *km, DPARMS *parms, const char *keystroke);

struct keymap {
   KEYCALL kc_func;
};



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
   int line_count;          ///< number of screen lines in region
   int chars_left;          ///< left margin
   int chars_count;         ///< number of characters to print per line
};



#endif

#ifndef PAGER_H
#define PAGER_H

#include <stdbool.h>

enum drive_values {
   PAGER_EXIT = 0,
   PAGER_SELECT,
   FOCUS_DOWN_ONE,
   FOCUS_UP_ONE,
   FOCUS_DOWN_PAGE,
   FOCUS_UP_PAGE,
   FOCUS_END,
   FOCUS_HOME,
   SCROLL_DOWN_ONE,
   SCROLL_UP_ONE,
   SCROLL_DOWN_PAGE,
   SCROLL_UP_PAGE,
   SCROLL_END,
   SCROLL_HOME
};


typedef struct pager_keymap PKMAP;
typedef struct display_params DPARMS;

typedef int (*pwb_print_line)(int row_index, int indicated, int length, void *data_source);

enum action_return_values {
   ARV_CONTINUE = 0,
   ARV_REPLOT_DATA,
   ARV_EXIT
};
typedef enum action_return_values ARV;

typedef ARV (*PACTION)(DPARMS*);

struct pager_keymap {
   int action_index;         ///< index into array of function pointers
   const char *capname;      ///< termcap name for key
   const char *value;        ///< string emitted when the key is pressed
};



struct display_params {
   int reserve_top;         ///< lines at top left alone
   int reserve_right;       ///< chars to right left alone
   int reserve_bottom;      ///< lines below left alone
   int reserve_left;        ///< chars to left left alone

   int index_row_top;       ///< index number of top row of source
   int index_row_focus;     ///< index of row that currently has focus'
   int row_count;           ///< number of rows in the source

   void *data_source;       ///< passed to print function for printing lines
   pwb_print_line printer;  ///< function pointer to be called for each output line
   const PACTION *actions;  ///< function table of actions to be performed on matching keystroke
   const PKMAP *keymap;     ///< 'table' associating keystrokes to indexes to a function
                            ///  table of actions

   int line_top;            ///< screen line to begin printing
   int line_count;          ///< number of screen lines in region
   int chars_left;          ///< left margin
   int chars_count;         ///< number of characters to print per line
};

extern PKMAP sel_keymap[];
extern PACTION const sel_actionmap[];

/**
 * @defgroup DPARMS_info Extract information from DPARMs instance
 * @{
 */
int get_index_bottom_limit(const DPARMS *parms);
int get_index_bottom_line(const DPARMS *parms);
bool row_index_is_visible(const DPARMS *parms, int row_index);
int get_line_index_from_row_index(const DPARMS *parms, int row_index);
/** @} */

void update_keymap_values(PKMAP *kmap, int el_size);
void update_display_params(DPARMS *params);
void print_indexed_row(const DPARMS *parms, int row_index, bool has_focus);
PACTION page_get_action(const DPARMS *parms, const char *keystroke);

void scroll_line_up(const DPARMS *parms);
void scroll_line_down(const DPARMS *parms);

void print_page(DPARMS *params);

void start_pager(DPARMS *params);




ARV pager_quit(DPARMS *parms);
ARV pager_activate(DPARMS *parms);

ARV pager_focus_down_one(DPARMS *parms);
ARV pager_focus_up_one(DPARMS *parms);
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

#endif

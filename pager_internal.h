#ifndef PAGER_INTERNAL_H
#define PAGER_INTERNAL_H


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
void print_indexed_row(const DPARMS *parms, int row_index, bool has_focus);
PACTION page_get_action(const DPARMS *parms, const char *keystroke);

void scroll_line_up(const DPARMS *parms);
void scroll_line_down(const DPARMS *parms);

void print_page(DPARMS *params);


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

extern PKMAP sel_keymap[];
extern PACTION const sel_actionmap[];

#endif

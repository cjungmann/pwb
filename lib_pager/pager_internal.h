#ifndef PAGER_INTERNAL_H
#define PAGER_INTERNAL_H

/**
 * @defgroup DPARMS_info Extract information from DPARMs instance
 * @{
 */
int get_index_bottom_line(const DPARMS *parms);
bool row_index_is_visible(const DPARMS *parms, int row_index);
int get_line_index_from_row_index(const DPARMS *parms, int row_index);
/** @} */

void print_indexed_row(const DPARMS *parms, int row_index, bool has_focus);
void print_page(DPARMS *params);


/**
 * @defgroup PAGER_ACTIONS Actions that can be triggered by user input
 * @{
 */
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

/** @} */

#endif

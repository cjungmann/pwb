#ifndef PAGER_H
#define PAGER_H

#include <stdbool.h>

typedef int (*pwb_print_line)(int row_index, int indicated, int length, void *data_source);

#include "pager_defs.h"
#include "pager_internal.h"

/**
 * @defgroup LIB_PUBLIC Public library functions
 * @brief Functions found in `pager_public.c`
 * @{
 */
void initialize_dparms(DPARMS *parms,
                       void *data_source,
                       int row_count,
                       pwb_print_line printer,
                       SCR_FUNCS *funcs
   );

void set_screen_margins(DPARMS *parms, int top, int right, int bottom, int left);
void print_page(DPARMS *params);
// void start_pager(DPARMS *parms);

/** @} */


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

#endif

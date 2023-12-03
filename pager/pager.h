#ifndef PAGER_H
#define PAGER_H

#include <stdbool.h>

#include "pager_defs.h"
#include "pager_internal.h"

extern PKMAP sel_keymap[];
extern PACTION const sel_actionmap[];

/**
 * @defgroup LIB_PUBLIC Public library functions
 * @brief Functions found in `pager_public.c`
 * @{
 */
void update_keymap_values(PKMAP *kmap, int el_size);
void initialize_dparms(DPARMS *parms, void *data_source, int row_count, pwb_print_line printer);
void set_screen_margins(DPARMS *parms, int top, int right, int bottom, int left);
void replace_keymmap(DPARMS *parms, PKMAP *new_map);

void start_pager(DPARMS *parms);

/** @} */

/**
 * @defgroup SCREEN_PUBLIC Text managing functions for line_printing
 *
 * These are a subset of functions from screen.h that are EXPORTed
 * @{
 */
void set_bold_mode(void);
void set_italic_mode(void);
void set_reverse_mode(void);
void set_normal_mode(void);

void start_standout_mode(void);
void stop_standout_mode(void);

void reset_screen(void);
void hide_cursor(void);
void show_cursor(void);
void normal_cursor(void);
/** @} */

/**
 * @defgroup KEYSTROKE_PUBLIC Public functions from get_keystroke.c module
 * @{
 */
char *transform_keystroke(char *buff, int bufflen, const char *keystroke, const char *esc_str);
char *transform_keystring(char *buff, int bufflen, const char *keystring, const char *esc_str);
char* get_keystroke(char *buff, int bufflen);
char* get_transformed_keystroke(char *buff, int bufflen, const char *esc_str);

/** @} */



#endif

#ifndef PWB_HANDLE_H
#define PWB_HANDLE_H

#include "pwb_builtin.h"
#include "pwb_result.h"

#include <pager.h>
#include <stdbool.h>

#define PWB_ID_STRING "pager with benefits"
#define WORD_LIST_INT_SIZE 12

typedef struct pwb_handle PWBH;

/**
 * @defgroup Bash add-ons:
 * Found in pwbh_support.c
 * @{
 */
#define pwbh_cell(var) (PWBH*)((var)->value)
bool pwbh_p(SHELL_VAR *var);
PWB_RESULT get_pwb_handle(PWBH **handle, const char *name);
void pwb_dispose_variable_value(SHELL_VAR *var);
int pwb_execute_command(WORD_LIST *wl);
/** @} */

int pwb_terminal_init(void);

struct pwb_handle {
   char idstring[sizeof(PWB_ID_STRING)];

   DPARMS dparms;

   /**
    * @brief The printer function word list has 6 elements:
    *        0. function name    - fixed value
    *        1. row index
    *        2. focus flag
    *        3. character limit
    *        4. data_source name - fixed value
    *        5. data_extra name  - fixed value
    */
   WORD_LIST *printer_wl;        //< Word-list to pass to print_function

   /**
    * @brief The exec function word_list has 5 elements:
    *        0. function name      - fixed value
    *        1. keystroke string
    *        2. current row number
    *        3. data_source name   - fixed value
    *        4. data_extra name    - fixed value
    */
   WORD_LIST *exec_wl;
};

int pwb_calc_handle_size(const char *data_source_name,
                         const char *printer_name,
                         const char *handle_name,
                         const char *exec_func_name,
                         const char *data_extra_name);

PWBH * pwb_initialize_handle(char *buffer,
                             int buffer_len,
                             const char *data_source_name,
                             int data_count,
                             const char *printer_name,
                             const char *handle_name,
                             const char *exec_name,
                             const char *data_extra_name);

// The following functions are found in pwbh_support.c
int pwb_raw_line_printer(int row_index,
                     int focus,
                     int length,
                     void *data_source,
                     void *data_extra);

int pwb_line_printer(int row_index,
                     int focus,
                     int length,
                     void *data_source,
                     void *data_extra);

int word_list_count(const WORD_LIST *wl);
int pwb_calc_word_list_size(int elements, int int_els);

void pwb_truncate_word_list_at_index(WORD_LIST *wl, int position);

void pwb_initialize_word_list_int_arg(WORD_LIST *wl, int position, char *buffer);
void pwb_set_word_list_string_arg(WORD_LIST *wl, int position, const char *string);
void pwb_set_word_list_int_val(WORD_LIST *wl, int position, int value);
void pwb_set_exec_keystroke_arg(WORD_LIST *wl, const char *string);

const char *pwbh_get_name_data_source(const PWBH * pwbh);
int pwbh_get_length_data_source(const PWBH *pwbh);
const char *pwbh_get_name_data_extra(const PWBH * pwbh);
const char *pwbh_get_name_printer(const PWBH * pwbh);
const char *pwbh_get_name_exec(const PWBH * pwbh);

void pwbh_print_set_row_index(PWBH *pwbh, int value);
void pwbh_print_set_focus(PWBH *pwbh, int value);
void pwbh_print_set_length(PWBH *pwbh, int value);

void pwbh_exec_set_row_number(PWBH *pwbh, int value);
void pwbh_exec_update_row_number(PWBH *pwbh);
void pwbh_exec_set_keystroke(PWBH *pwbh, const char *keystroke);
void pwbh_calc_borders(PWBH *pwbh);

PWBH *pwb_get_handle_from_name(const char *handle_name);


#endif

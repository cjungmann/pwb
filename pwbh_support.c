#include "pwb_builtin.h"
#include "pwb_handle.h"

// Shadow curses `trace` to hide from bash `trace`
#define trace trace_curses
#include <curses.h>
#include <term.h>
// remove shadow to continue
#undef trace

#include <assert.h>
#include <stdio.h>
#include <contools.h>
#include <pager.h>

/**
 * @brief Dispose of item referenced by SHELL_VAR::value.
 * @param "var"   variable whose contents are to be disposed of.
 *
 * Does the job of Bash's unavailable internal function,
 * dispose_variable_value.  Frees the memory pointed to by value
 * member according to its type.  The `value` member of the
 * SHELL_VAR will be NULL when done.
 */
void pwb_dispose_variable_value(SHELL_VAR *var)
{
   if (array_p(var))
      array_dispose(array_cell(var));
   else if (assoc_p(var))
      assoc_dispose(assoc_cell(var));
   else if (function_p(var))
      dispose_command(function_cell(var));
   else if (nameref_p(var))
      FREE(nameref_cell(var));
   else
      FREE(value_cell(var));

   var->value = NULL;
}


bool pwbh_p(SHELL_VAR *var)
{
   if (specialvar_p(var))
   {
      PWBH *pwbh = pwbh_cell(var);
      return strcmp(pwbh->idstring, PWB_ID_STRING)==0;
   }

   return false;
}

PWB_RESULT get_pwb_handle(PWBH **handle, const char *name)
{
   PWB_RESULT result = PWB_FAILURE;
   *handle = NULL;

   SHELL_VAR *sv = find_variable(name);
   if (sv)
   {
      if (pwbh_p(sv))
      {
         *handle = pwbh_cell(sv);
         result = PWB_SUCCESS;
      }
      else
         result = PWB_INVALID_HANDLE;
   }
   else
      result = PWB_INVALID_VARIABLE;

   return result;
}

/**
 * @defgroup MARGIN_RELATIVE_POSITIONING
 * @{
 */

bool pwbh_position_to_head(PWBH *pwbh)
{
   int top_lines = pwbh->dparms.margin_top;
   if (top_lines > 0)
   {
      ti_set_cursor_position(0, pwbh->dparms.margin_left);
      return true;
   }

   return false;
}

bool pwbh_position_to_foot(PWBH *pwbh)
{
   int bottom_lines = pwbh->dparms.margin_bottom;
   if (bottom_lines > 0)
   {
      int pos_foot = pwbh->dparms.line_count - bottom_lines - 1;
      ti_set_cursor_position(pos_foot, pwbh->dparms.margin_left);
      return true;
   }

   return false;
}

/** @} */

/**
 * Terminal setup stuff
 */
int pwb_terminal_init(void)
{
   int error_return;
   int result = setupterm((char*)NULL, STDIN_FILENO, &error_return);
   if (result)
   {
      switch(error_return)
      {
         case 1: printf("curses unavailable in hardcopy mode.\n"); break;
         case 0: printf("curses unavailable in generic terminal.\n"); break;
         case -1: printf("curses unavailable, terminfo database missing.\n"); break;
      }
   }
   return result;
}

int pwb_execute_command(WORD_LIST *wl)
{
   int cmd_flags = CMD_INHIBIT_EXPANSION | CMD_STDPATH;

   COMMAND *command = make_bare_simple_command();
   command->value.Simple->words = wl;
   command->value.Simple->redirects = (REDIRECT*)NULL;
   command->flags = command->value.Simple->flags = cmd_flags;

   return execute_command(command);
}

// Prototype to make function available in pwb_line_printer
// without moving it from near related functions.
void pwbh_print_set_shell_function(PWBH *pwbh, const char *func_name);

/**
 * @brief Default auto-hilighting printer function for DPARMS
 */
int pwb_line_printer(int row_index,
                     int focus,
                     int length,
                     void *data_source,
                     void *data_extra)
{
   PWBH *ph = (PWBH*)data_extra;

   // update WORD_LIST values:
   pwbh_print_set_row_index(ph, row_index);
   pwbh_print_set_focus(ph, focus);

   if (focus)
      start_standout_mode();

   int result = pwb_execute_command(ph->printer_wl);

   if (focus)
   {
      stop_standout_mode();
      if (ph->print_func_head || ph->print_func_foot)
      {
         if (ph->print_func_head && pwbh_position_to_head(ph))
         {
            pwbh_print_set_shell_function(ph, ph->print_func_head);
            result = pwb_execute_command(ph->printer_wl);
         }

         if (ph->print_func_foot && pwbh_position_to_foot(ph))
         {
            pwbh_print_set_shell_function(ph, ph->print_func_foot);
            result = pwb_execute_command(ph->printer_wl);
         }

         pwbh_print_set_shell_function(ph, NULL);
      }
   }

   return result;
}

/**
 * @brief Alternate non-hilighting printer function for DPARMS
 */
int pwb_raw_line_printer(int row_index,
                         int focus,
                         int length,
                         void *data_source,
                         void *data_extra)
{
   PWBH *ph = (PWBH*)data_extra;

   // update WORD_LIST values:
   pwbh_print_set_row_index(ph, row_index);
   pwbh_print_set_focus(ph, focus);

   return pwb_execute_command(ph->printer_wl);
}

/**
 * @brief General function to print either header or footer
 */
int pwb_margin_printer(PWBH *handle, bool header)
{
   int result = PWB_SUCCESS;

   // Set function-wide DPARMS values
   DPARMS *dparms = &handle->dparms;
   pwbh_print_set_row_index(handle, dparms->index_row_focus);

   int col_to_start = dparms->chars_left;
   int line_to_start, lines_to_print;

   if (header)
   {
      line_to_start = 0;
      lines_to_print = dparms->margin_top;
   }
   else
   {
      line_to_start = dparms->line_bottom;
      lines_to_print = dparms->line_count;
   }

   int margin_line = line_to_start;


   for (int i=0; result==PWB_SUCCESS && i < lines_to_print; ++i)
   {
      ti_set_cursor_position(margin_line, col_to_start);
      pwbh_print_set_focus(handle, i);
      result = pwb_execute_command(handle->printer_wl);

      ++margin_line;
   }

   return result;
}

/**
 * @brief For assertions to ensure a index is not out-of-range
 *
 * Forget about the WORD_LIST being an array, count the elements
 * using the linked list pointers.
 */
int word_list_count(const WORD_LIST *wl)
{
   int count = 0;
   while (wl)
   {
      ++count;
      wl = wl->next;
   }
   return count;
}


/**
 * @brief Calculate memory requirements for WORD_LIST of @p elements elements
 *        and @p int_els integer vaules.
 */
int pwb_calc_word_list_size(int elements, int int_els)
{
   return elements * (sizeof(WORD_LIST) + sizeof(WORD_DESC))
      + int_els * WORD_LIST_INT_SIZE;
}

void pwb_truncate_word_list_at_index(WORD_LIST *wl, int position)
{
   assert(position < word_list_count(wl));
   wl[position].next = NULL;
}

/**
 * @brief Set WORD_DESC::word to buffer at specified WORD_LIST position
 *
 * Important note: the WORD_LIST is both and array and a linked list.
 * We exploit the array arrangement to directly find the correct WORD_DESC
 */
void pwb_initialize_word_list_int_arg(WORD_LIST *wl, int position, char *buffer)
{
   assert(position < word_list_count(wl));
   WORD_DESC *wd = wl[position].word;
   wd->word = buffer;
   // Set string value to '0' (remember it's now a string of \0):
   *buffer = '0';
}

/**
 * @brief Set the WORD_DESC::word pointer for the specified WORD_LIST argument
 *
 * Important note: the WORD_LIST is both and array and a linked list.
 * We exploit the array arrangement to directly find the correct WORD_DESC
 */
void pwb_set_word_list_string_arg(WORD_LIST *wl, int position, const char *string)
{
   assert(position < word_list_count(wl));
   WORD_DESC *wd = wl[position].word;
   wd->word = (char*)string;
}

/**
 * @brief Write the string value of an integer to the WORD_DESC::word buffer
 * for the specified WORD_LIST argument.
 *
 * Important note: the WORD_LIST is both and array and a linked list.
 * We exploit the array arrangement to directly find the correct WORD_DESC
 */
void pwb_set_word_list_int_val(WORD_LIST *wl, int position, int value)
{
   assert(position < word_list_count(wl));
   char *buff = wl[position].word->word;
   int written = snprintf(buff, WORD_LIST_INT_SIZE, "%d", value);
   assert(written < WORD_LIST_INT_SIZE);
}

/**
 * @brief Access to data_source name in DPARMS
 */
const char *pwbh_get_name_data_source(const PWBH * pwbh)
{
   return (char*)pwbh->dparms.data_source;
}

int pwbh_get_length_data_source(const PWBH *pwbh)
{
   return pwbh->dparms.row_count;
}


/**
 * @brief Access to data_extra name in DPARMS
 */
const char *pwbh_get_name_data_extra(const PWBH * pwbh)
{
   return (char*)pwbh->dparms.data_extra;
}

/**
 * @brief Access to shell printer function name in printer word_list
 */
const char *pwbh_get_name_printer(const PWBH * pwbh)
{
   return pwbh->printer_wl->word->word;
}

/**
 * @brief Access to shell exec function name in exec word_list
 */
const char *pwbh_get_name_exec(const PWBH * pwbh)
{
   if (pwbh->exec_wl)
      return pwbh->exec_wl->word->word;
   else
      return NULL;
}

void pwbh_print_set_shell_function(PWBH *pwbh, const char *func_name)
{
   // Make it easy to return to default (mandatory) print func
   if (func_name == NULL)
      func_name = pwbh->print_func_line;

   pwb_set_word_list_string_arg(pwbh->printer_wl, 0, func_name);
}


void pwbh_print_set_row_index(PWBH *pwbh, int value)
{
   pwb_set_word_list_int_val(pwbh->printer_wl, 1, value);
}

void pwbh_print_set_length(PWBH *pwbh, int value)
{
   pwb_set_word_list_int_val(pwbh->printer_wl, 3, value);
}

void pwbh_print_set_focus(PWBH *pwbh, int value)
{
   pwb_set_word_list_int_val(pwbh->printer_wl, 4, value);
}

void pwbh_exec_set_row_number(PWBH *pwbh, int value)
{
   pwb_set_word_list_int_val(pwbh->exec_wl, 2, value);
}

/**
 * @brief Set the row_number WORD_LIST param to the focus row
 */
void pwbh_exec_update_row_number(PWBH *pwbh)
{
   pwb_set_word_list_int_val(pwbh->exec_wl, 2, pwbh->dparms.index_row_focus);
}

void pwbh_exec_set_keystroke(PWBH *pwbh, const char *keystroke)
{
   pwb_set_word_list_string_arg(pwbh->exec_wl, 1, keystroke);
}

void pwbh_calc_borders(PWBH *pwbh)
{
   DPARMS *dparms = &pwbh->dparms;
   pager_calc_borders(dparms);
   pwbh_print_set_length(pwbh, dparms->chars_count);
}

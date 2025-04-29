#include "pwb_actions.h"
#include <argeater.h>
#include <pager.h>
#include <contools.h>
#include <stdio.h>
#include <stdlib.h>    // for malloc/free
#include <alloca.h>
#include <math.h>      // for log10

#include "pwb_handle.h"
#include "pwb_errors.h"
#include "pwb_argeater.h"
#include "pwb_keymap.h"
#include "pwb_utilities.h"

void print_the_word_list(WORD_LIST *wl)
{
   WORD_LIST *ptr = wl;
   int count=0;
   while (ptr)
   {
      printf("%d: %s.\n", ++count, ptr->word->word);
      ptr = ptr->next;
   }
}

PWB_RESULT pwb_action_dummy(PWBH *handle, ACLONE *args)
{
   printf("In the non-operational function, placeholder_runner.\n");
   return PWB_SUCCESS;
}

PWB_RESULT pwb_action_version(PWBH *handle, ACLONE *args)
{
   printf("pwb version 0.0.9\n");
   return PWB_SUCCESS;
}

/**
 * @brief Prints visible characters of string to limit in first argument
 *
 * Note that this function will get a NULL handle value because it's
 * a 'waives_handle' function.
 *
 * This function will print characters found in the string, but only
 * count those NOT IN a CSI sequence.  Trailing CSI sequences will be
 * printed, even after the maximum visible characters are printed, to
 * ensure cleanup CSI will be sent to the terminal.
 */
PWB_RESULT pwb_action_limit_print(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_FAILURE;

   unsigned int len = -1;
   const char *string = NULL;
   AE_ITEM items[] = {
      { (const char **)&len, "length", '\0', AET_ARGUMENT,
        "maximum screen characters to print", NULL, pwb_argeater_unsigned_int_setter },
      { &string, "string", '\0', AET_ARGUMENT,
        "string to print" }
   };

   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      if (string==NULL)
         (*error_sink)("missing string to print");
      else if (len < 0)
         (*error_sink)("Invalid string length (%d)", len);

      result = PWB_SUCCESS;
      if (len > 0)
      {
         bool in_csi = false;
         int count = 0;
         const char *ptr = string;

         while (*ptr)
         {
            if (in_csi)
            {
               // Letter terminates a CSI, we're not discriminating beyond that
               if ((*ptr>='a' && *ptr<='z') || (*ptr>='A' && *ptr<='Z'))
                  in_csi = false;
               // Only numerals and ';' in CSI expression, otherwise it's an error
               else if (!((*ptr>='0' && *ptr <='9') || *ptr==';'))
               {
                  (*error_sink)("at char position %d, unexpected character '%c'"
                                " (%d) in CSI expression",
                                ptr - string, *ptr, *ptr);
                  result = PWB_FAILURE;
                  break;
               }
               // We're not counting 'em, just print and move along
               putc(*ptr, stdout);
               ++ptr;
               continue;
            }
            // Check if we're starting a CSI
            else if (*ptr=='\x1b')
            {
               // Abort if escape does not start CSI sequence
               if (*(ptr+1)!='[')
               {
                  (*error_sink)("ate char position %d, unexpected character '%c' "
                                " (%d) following an escape character",
                                ptr - string, *ptr, *ptr);
                  result = PWB_FAILURE;
                  break;
               }

               putc(*ptr++, stdout);
               putc(*ptr++, stdout);
               in_csi = true;
               continue;
            }

            if (count < len)
               putc(*ptr, stdout);

            ++count;
            ++ptr;
         }
      }
   }

   return result;
}

PWB_RESULT pwb_action_measure_string(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_FAILURE;

   const char *var_name="PWB_VALUE";
   const char *string=NULL;
   AE_ITEM items[] = {
      { &string, "string", '\0', AET_ARGUMENT,
        "string to measure" },
      { &var_name, "var", 'v', AET_VALUE_OPTION,
        "Alternate to 'PWB_VALUE' for reporting result" }
   };

   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      if (string==NULL)
         (*error_sink)("missing string to measure");
      else
      {
         bool in_csi = false;
         int count = 0;
         const char *ptr = string;
         while (*ptr)
         {
            if (in_csi)
            {
               // Letter terminates a CSI, we're not discriminating beyond that
               if ((*ptr>='a' && *ptr<='z') || (*ptr>='A' && *ptr<='Z'))
                  in_csi = false;
               // Only numerals and ';' in CSI expression, otherwise it's an error
               else if (!((*ptr>='0' && *ptr <='9') || *ptr==';'))
               {
                  (*error_sink)("at char position %d, unexpected character '%c'"
                                " (%d) in CSI expression",
                                ptr - string, *ptr, *ptr);
                  break;
               }
               ++ptr;
               continue;
            }
            // Check if we're starting a CSI
            else if (*ptr=='\x1b')
            {
               // Abort if escape does not start CSI sequence
               if (*(ptr+1)!='[')
               {
                  (*error_sink)("ate char position %d, unexpected character '%c' "
                                " (%d) following an escape character",
                                ptr - string, *ptr, *ptr);
                  break;
               }

               ptr+=2;
               in_csi = true;
               continue;
            }

            // Don't let the increment languish under a conditional,
            // or we'll never get out of here:
            ++ptr;
            ++count;
         }

         // Don't attempt log10 on a zero value, but
         // make sure there's room for a '0'
         int num_length = (count==0?1:(floor(log10(count))+1)) + 1;

         char *buff = xmalloc(num_length);
         if (buff)
         {
            SHELL_VAR *sv = find_variable(var_name);
            if (!sv)
               sv = bind_variable(var_name, "", 0);

            if (sv)
            {
               snprintf(buff, num_length, "%d", count);
               pwb_dispose_variable_value(sv);
               sv->value = buff;
               if (invisible_p(sv))
                  VUNSETATTR(sv, att_invisible);
               result = PWB_SUCCESS;
            }
         }
      }
   }

   return result;
}

PWB_RESULT pwb_action_get_keystroke(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_SUCCESS;

   const char *var_name="PWB_VALUE";
   AE_ITEM items[] = {
      { &var_name, "var", 'v', AET_VALUE_OPTION,
        "Alternate to 'PWB_VALUE' for reporting result" }
   };

   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      const char *keys = get_keystroke(NULL, 0);
      if (keys)
      {
         int keylen = strlen(keys);
         if (keylen > 0)
         {
            SHELL_VAR *sv = find_variable(var_name);
            if (!sv)
               sv = bind_variable(var_name, "", 0);

            if (sv)
            {
               pwb_dispose_variable_value(sv);
               sv->value = savestring(keys);

               if (invisible_p(sv))
                  VUNSETATTR(sv, att_invisible);
               result = PWB_SUCCESS;
            }
         }
      }
   }

   return result;
}

PWB_RESULT pwb_action_audit_var(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_SUCCESS;

   const char *var_name = NULL;
   const char *var_output = "PWB_VALUE";
   bool include_context = false;
   bool include_attributes = false;

   AE_ITEM items[] = {
      { &var_name, "name", '\0', AET_ARGUMENT,
        "name of variable whose attributes are to be revealed" },
      { &var_output, "var", 'v', AET_VALUE_OPTION,
        "Alternate to 'PWB_VALUE' for reporting result" },
      { (const char**)&include_attributes, "attributes", 'a', AET_FLAG_OPTION,
        "Include variable attributes", NULL, argeater_bool_setter },
      { (const char **)&include_context, "context", 'c', AET_FLAG_OPTION,
        "Include variable context number", NULL, argeater_bool_setter }
   };

   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      SHELL_VAR *sv_out = find_variable(var_output);
      if (!sv_out)
         sv_out = bind_variable(var_output, "", 0);

      if (!sv_out)
      {
         (*error_sink)("unable to secure variable '%s'", var_output);
         result = PWB_FAILURE;
      }
      else
      {
         // Remove residual value
         pwb_dispose_variable_value(sv_out);

         SHELL_VAR *sv = find_variable(var_name);
         if (sv)
         {
            // Collect and report important attributes:
            int bufflen = get_var_parameters(NULL,
                                             0,
                                             sv,
                                             include_context,
                                             include_attributes);
            char *buff = xmalloc(bufflen);

            if (bufflen == get_var_parameters(buff,
                                              bufflen,
                                              sv,
                                              include_context,
                                              include_attributes))
            {
               // Attach report to output variable:
               sv_out->value = buff;
               if (invisible_p(sv_out))
                  VUNSETATTR(sv_out, att_invisible);
               result = PWB_SUCCESS;
            }
            else
            {
               sv_out->value = savestring("error");
               (*error_sink)("Miscalculated memory requirements for '%s'", var_name);
               result = PWB_FAILURE;
            }
         }
         else
         {
            sv_out->value = savestring("error");
            (*error_sink)("variable '%s' is unavailable", var_name);
            result = PWB_FAILURE;
         }
      }
   }

   return result;
}


PWB_RESULT pwb_action_init(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_SUCCESS;

   pwb_terminal_init();
   pager_init();
   ti_hide_cursor();

   return result;
}

PWB_RESULT pwb_action_restore(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_SUCCESS;
   pager_cleanup();
   ti_show_cursor();
   return result;
}

PWB_RESULT pwb_action_declare(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_FAILURE;

   const char *handle_name = NULL;
   const char *data_source = NULL;
   int data_count = -1;
   const char *print_func = NULL;
   const char *exec_func = NULL;
   const char *extra_data = NULL;
   const char *use_non_hilite_print = NULL;
   const char *head_print_func = NULL;
   const char *foot_print_func = NULL;
   const char *left_print_func = NULL;
   const char *right_print_func = NULL;
   AE_ITEM items[] = {
      { &handle_name, "handle_name", '\0', AET_ARGUMENT,
      "name of new handle" },

      { &data_source, "data_source", '\0', AET_ARGUMENT,
        "name of data source" },

      { (const char **)&data_count, "data_count", '\0', AET_ARGUMENT,
        "lines in data source", NULL, pwb_argeater_int_setter },

      { &print_func, "printer", '\0', AET_ARGUMENT,
        "name of line print function", NULL, pwb_argeater_function_setter },

      { &exec_func, "exec", 'e', AET_VALUE_OPTION,
        "name of line execute function", NULL, pwb_argeater_function_setter},

      { &extra_data, "extra_data", 'd', AET_VALUE_OPTION,
        "name of extra data variable" },

      { &head_print_func, "top_printer", 't', AET_VALUE_OPTION,
        "name of function to print top margin",
        NULL, pwb_argeater_function_setter },

      { &foot_print_func, "bottom_printer", 'b', AET_VALUE_OPTION,
        "name of function to print bottom margin",
        NULL, pwb_argeater_function_setter },

      { &left_print_func, "left_printer", 'l', AET_VALUE_OPTION,
        "name of function to print left margin",
        NULL, pwb_argeater_function_setter },

      { &right_print_func, "right_printer", 'r', AET_VALUE_OPTION,
        "name of function to print right margin",
        NULL, pwb_argeater_function_setter },

      { &use_non_hilite_print, "no_hilites", 'h', AET_FLAG_OPTION,
        "do not auto-hilite focus lines" }
   };

   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      // Validate a few parameters
      if ( data_count < 0 )
      {
         (*error_sink)("PWB action 'declare' requires a non-negative data count value.");
         goto early_exit;
      }
      if ( print_func == NULL || handle_name == NULL || data_source == NULL )
      {
         (*error_sink)("PWB action 'declare' requires a handle, a data_source, and a print function.");
         goto early_exit;
      }

      int handle_size = pwb_calc_handle_size(data_source,
                                             print_func,
                                             handle_name,
                                             exec_func,
                                             extra_data,
                                             head_print_func,
                                             foot_print_func,
                                             left_print_func,
                                             right_print_func);
      char *buff = (char*)xmalloc(handle_size);
      if (buff)
      {
         __attribute__((unused))
            PWBH *pwbh = pwb_initialize_handle(buff, handle_size,
                                               data_source,
                                               data_count,
                                               print_func,
                                               handle_name,
                                               exec_func,
                                               extra_data,
                                               head_print_func,
                                               foot_print_func,
                                               left_print_func,
                                               right_print_func);

         WORD_LIST *wl_ptr = pwbh->printer_wl;
         __attribute__((unused))
         WORD_LIST *wl_length = &wl_ptr[3];

         if (use_non_hilite_print)
            pwbh->disable_hiliting = true;

         SHELL_VAR *sv = NULL;
         if (variable_context == 0)
            sv = bind_variable(handle_name, "", 0);
         else
            sv = make_local_variable(handle_name, 0);

         if (sv)
         {
            pwb_dispose_variable_value(sv);
            sv->value = buff;
            VSETATTR(sv, att_special);
            if (invisible_p(sv))
               VUNSETATTR(sv, att_invisible);

            // Initialize the terminal and pager as soon
            // as we know everything is gonna be OK:
            pwb_terminal_init();
            pager_init();

            result = PWB_SUCCESS;
         }
      }
      else
         (*error_sink)("Out of memory for creating a new PWB handle.");
   }

     early_exit:
   return result;
}

PWB_RESULT pwb_action_report(PWBH *handle, ACLONE *args)
{
   printf("In pwb_action_report!\n");
   PWB_RESULT result = PWB_FAILURE;
   if (handle)
   {
      printf("      data source: %s\n", pwbh_get_name_data_source(handle));
      printf("        row count: %d\n", pwbh_get_length_data_source(handle));
      printf("     printer name: %s\n", pwbh_get_name_printer(handle));

      printf("\nline printer arguments\n");
      print_the_word_list(handle->printer_wl);

      if (handle->exec_wl)
      {
         printf("\nexec_function arguments\n");
         print_the_word_list(handle->exec_wl);
      }

      result = PWB_SUCCESS;
   }

   return result;
}

PWB_RESULT pwb_action_trigger(PWBH * handle, ACLONE *args)
{
   PWB_RESULT result = PWB_FAILURE;
   WORD_LIST *word_list = handle->exec_wl;

   unsigned int keymap_index = -1;
   const char *keystroke = NULL;
   AE_ITEM items[] = {
      { (const char **)&keymap_index, "keymap_index", '\0', AET_ARGUMENT,
        "keymap action index", NULL, pwb_argeater_unsigned_int_setter },
      { &keystroke, "keystroke", '\0', AET_ARGUMENT,
        "optional keystroke string" }
   };

   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      // Last action in array calls the exec function.  Don't proceed
      // unless the exec function has been declared (test by checking
      // for the WORD_LIST handle member).
      if (keymap_index == action_table_count-1)
      {
         if (word_list)
            pwbh_exec_set_keystroke(handle, keystroke);
         else
         {
            (*error_sink)("Attempted to call undeclared exec function.");
            result = PWB_EXEC_FUNCTION_UNDECLARED;
            goto early_exit;
         }
      }

      PWB_KEYACT action = get_keyact(keymap_index);
      if (action)
      {
         // Make word_list from arguments
         result = (*action)(handle, word_list);
      }
      else
      {
         (*error_sink)("Index %d is out-of-range (0...%d).",
                       keymap_index,
                       action_table_count);

         result = PWB_INVALID_ARGUMENT;
      }
   }

early_exit:
   return result;
}


/* pwb_action_start() in its own source file */

PWB_RESULT pwb_action_set_margins(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_SUCCESS;

   int top = -1;
   int right = -1;
   int bottom = -1;
   int left = -1;
   AE_ITEM items[] = {
      { (const char **)&top, "top_margin", '\0', AET_ARGUMENT,
        "value of left margin", NULL, pwb_argeater_int_setter },

      { (const char **)&right, "right_margin", '\0', AET_ARGUMENT,
        "value of left margin", NULL, pwb_argeater_int_setter },

      { (const char **)&bottom, "bottom_margin", '\0', AET_ARGUMENT,
        "value of left margin", NULL, pwb_argeater_int_setter },

      { (const char **)&left, "left_margin", '\0', AET_ARGUMENT,
        "value of left margin", NULL, pwb_argeater_int_setter }
   };

   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      if (!pager_set_margins(&handle->dparms, top, right, bottom, left))
      {
         (*error_sink)("Impossible margins request.");
         result = PWB_FAILURE;
      }
   }

   return result;
}

PWB_RESULT pwb_action_plot_header(PWBH *handle, ACLONE *pargs)
{
   PWB_RESULT result = PWB_FAILURE;
   if (!pwb_margin_printer(handle, true))
       result = 0;
   return result;
}

PWB_RESULT pwb_action_plot_line(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_FAILURE;

   int row = -1;
   AE_ITEM items[] = {
      { (const char **)&row, "data_row", '\0', AET_ARGUMENT,
        "index of row to replot", NULL, pwb_argeater_int_setter }
   };
   AE_MAP map = INIT_MAP(items);

   if (argeater_process(args, &map))
   {
      if (row >= 0)
      {
         // pager_plot_row() calls function
         pager_plot_row(&handle->dparms, row);
         result = ARV_CONTINUE;
      }
   }
   return result;
}

PWB_RESULT pwb_action_plot_screen(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_SUCCESS;
   pwbh_calc_borders(handle);
   pager_plot(&handle->dparms);
   return result;
}

PWB_RESULT pwb_action_get_data_source(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_SUCCESS;

   const char *var_name="PWB_VALUE";
   AE_ITEM items[] = {
      { &var_name, "var", 'v', AET_VALUE_OPTION,
        "Alternate to 'PWB_VALUE' for reporting result" }
   };

   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      SHELL_VAR *sv = find_variable(var_name);
      if (!sv)
         sv = bind_variable(var_name, "", 0);

      if (sv)
      {
         pwb_dispose_variable_value(sv);
         sv->value = savestring(pwbh_get_name_data_source(handle));

         if (invisible_p(sv))
            VUNSETATTR(sv, att_invisible);
         result = PWB_SUCCESS;
      }
   }

   return result;
}

PWB_RESULT pwb_action_get_data_count(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_SUCCESS;

   int count = handle->dparms.row_count;
   // +1 for count of digits, +1 for terminating /0
   int num_length = (count==0?1:(floor(log10(count))+1)) + 1;
   char *buff = alloca(num_length);
   snprintf(buff, num_length, "%d", count);

   const char *var_name="PWB_VALUE";
   AE_ITEM items[] = {
      { &var_name, "var", 'v', AET_VALUE_OPTION,
        "Alternate to 'PWB_VALUE' for reporting result" }
   };

   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      SHELL_VAR *sv = find_variable(var_name);
      if (!sv)
         sv = bind_variable(var_name, "", 0);

      if (sv)
      {
         pwb_dispose_variable_value(sv);
         sv->value = savestring(buff);

         if (invisible_p(sv))
            VUNSETATTR(sv, att_invisible);
         result = PWB_SUCCESS;
      }
   }

   return result;
}

PWB_RESULT pwb_action_set_data_count(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_SUCCESS;

   int new_count = 0;
   AE_ITEM items[] = {
      { (const char **)&new_count, "count", '\0', AET_ARGUMENT,
        "New value to use for the data count",
        NULL, argeater_int_setter }
   };
   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      handle->dparms.row_count = new_count;
   }

   return result;
}

PWB_RESULT pwb_action_get_top_row(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_SUCCESS;

   int value = handle->dparms.index_row_top;
   // +1 for count of digits, +1 for terminating /0
   int num_length = (value==0?1:(floor(log10(value))+1)) + 1;
   char *buff = alloca(num_length);
   snprintf(buff, num_length, "%d", value);

   const char *var_name="PWB_VALUE";
   AE_ITEM items[] = {
      { &var_name, "var", 'v', AET_VALUE_OPTION,
        "Alternate to 'PWB_VALUE' for reporting result" }
   };

   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      SHELL_VAR *sv = find_variable(var_name);
      if (!sv)
         sv = bind_variable(var_name, "", 0);

      if (sv)
      {
         pwb_dispose_variable_value(sv);
         sv->value = savestring(buff);

         if (invisible_p(sv))
            VUNSETATTR(sv, att_invisible);
         result = PWB_SUCCESS;
      }
   }

   return result;
}

PWB_RESULT pwb_action_get_focus_row(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_SUCCESS;

   int value = handle->dparms.index_row_focus;
   // +1 for count of digits, +1 for terminating /0
   int num_length = (value==0?1:(floor(log10(value))+1)) + 1;
   char *buff = alloca(num_length);
   snprintf(buff, num_length, "%d", value);

   const char *var_name="PWB_VALUE";
   AE_ITEM items[] = {
      { &var_name, "var", 'v', AET_VALUE_OPTION,
        "Alternate to 'PWB_VALUE' for reporting result" }
   };

   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      SHELL_VAR *sv = find_variable(var_name);
      if (!sv)
         sv = bind_variable(var_name, "", 0);

      if (sv)
      {
         pwb_dispose_variable_value(sv);
         sv->value = savestring(buff);

         if (invisible_p(sv))
            VUNSETATTR(sv, att_invisible);
         result = PWB_SUCCESS;
      }
   }

   return result;
}

PWB_RESULT pwb_action_set_focus_row(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_SUCCESS;

   int index_focus = -1;
   int index_top = -1;

   AE_ITEM items[] = {
      { (const char **)&index_focus, "focus", 'f', AET_ARGUMENT,
        "index of desired focus row", NULL, pwb_argeater_int_setter },
      { (const char **)&index_top, "top", 't', AET_ARGUMENT,
        "index of desired top row", NULL, pwb_argeater_int_setter },
   };

   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      if ( index_focus >  handle->dparms.row_count )
      {
         (*error_sink)("Requested focus index %d is out of range of %d rows.",
                       index_focus, handle->dparms.row_count );
         result = PWB_FAILURE;
      }
      if ( index_top >  handle->dparms.row_count )
      {
         (*error_sink)("Requested top index %d is out of range of %d rows.",
                       index_top, handle->dparms.row_count );
         result = PWB_FAILURE;
      }

      pager_set_focus(&handle->dparms, index_focus, index_top);
   }

   return result;
}

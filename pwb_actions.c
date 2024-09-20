#include "pwb_actions.h"
#include <argeater.h>
#include <pager.h>
#include <contools.h>
#include <stdio.h>
#include <stdlib.h>    // for malloc/free
#include <alloca.h>

#include "pwb_handle.h"
#include "pwb_errors.h"
#include "pwb_argeater.h"
#include "pwb_keymap.h"

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

   static const char *handle_name = NULL;
   static const char *data_source = NULL;
   static int data_count = -1;
   static const char *print_func = NULL;
   static const char *exec_func = NULL;
   static const char *extra_data = NULL;
   static const char *use_non_hilite_print = NULL;
   static const char *head_print_func = NULL;
   static const char *foot_print_func = NULL;
   static AE_ITEM items[] = {
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
      { &use_non_hilite_print, "no_hilites", 'h', AET_FLAG_OPTION,
        "do not auto-hilite focus lines" }
   };

   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      int handle_size = pwb_calc_handle_size(data_source,
                                             print_func,
                                             handle_name,
                                             exec_func,
                                             extra_data,
                                             head_print_func,
                                             foot_print_func);
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
                                               foot_print_func);

         WORD_LIST *wl_ptr = pwbh->printer_wl;
         __attribute__((unused))
         WORD_LIST *wl_length = &wl_ptr[3];

         // Kinda kludgy, but easier to just switch here if requesting
         // no-hilite version than adding parameters to pwb_initialize_handle:
         if (use_non_hilite_print)
            pwbh->dparms.printer = pwb_raw_line_printer;

         SHELL_VAR *sv = NULL;
         if (variable_context == 0)
            sv = bind_variable(handle_name, "", 0);
         else
            sv = make_local_variable(handle_name, 0);

         if (sv)
         {
            pwb_dispose_variable_value(sv);
            sv->value = buff;
            sv->attributes |= att_special;

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

   static unsigned int keymap_index = -1;
   static const char *keystroke = NULL;
   static AE_ITEM items[] = {
      { (const char **)&keymap_index, "keymap_index", '\0', AET_ARGUMENT,
        "keymap action index", NULL, pwb_argeater_unsigned_int_setter },
      { &keystroke, "keystroke", '\0', AET_ARGUMENT,
        "optional keystroke string" }
   };

   static AE_MAP map = INIT_MAP(items);
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

   static int top = -1;
   static int right = -1;
   static int bottom = -1;
   static int left = -1;
   static AE_ITEM items[] = {
      { (const char **)&top, "top_margin", '\0', AET_ARGUMENT,
        "value of left margin", NULL, pwb_argeater_int_setter },

      { (const char **)&right, "right_margin", '\0', AET_ARGUMENT,
        "value of left margin", NULL, pwb_argeater_int_setter },

      { (const char **)&bottom, "bottom_margin", '\0', AET_ARGUMENT,
        "value of left margin", NULL, pwb_argeater_int_setter },

      { (const char **)&left, "left_margin", '\0', AET_ARGUMENT,
        "value of left margin", NULL, pwb_argeater_int_setter }
   };

   static AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
      pager_set_margins(&handle->dparms, top, right, bottom, left);

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

   static int row = -1;
   static AE_ITEM items[] = {
      { (const char **)&row, "data_row", '\0', AET_ARGUMENT,
        "index of row to replot", NULL, pwb_argeater_int_setter }
   };
   static AE_MAP map = INIT_MAP(items);

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
   pager_plot(&handle->dparms);
   return result;
}

PWB_RESULT pwb_action_erase_head(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_FAILURE;
   DPARMS *dparms = &handle->dparms;
   if (dparms->margin_top > 0)
   {
      int pos_left = dparms->margin_left + 1;
      // put cursor at top/left of top margin
      ti_printf("\x1b[1;%dH", pos_left);

      // print empty lines for each row
      for (int i=0; i<dparms->margin_top; ++i)
      {
         // Erase chars-count characters on this line
         ti_printf("\x1b[%dX", dparms->chars_count);
         // Down one to next line
         ti_printf("\x1b[1B");
         ti_printf("\x1b[%G", pos_left);
      }

      // Leave with cursor at top/left
      ti_printf("\x1b[1;%dH", pos_left);

      result = PWB_SUCCESS;
   }

   return result;
}

PWB_RESULT pwb_action_erase_foot(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_FAILURE;
   DPARMS *dparms = &handle->dparms;
   if (dparms->margin_bottom > 0)
   {
      // Make sure cursor is below scroll region to prevent
      // downward cursor movement from scrolling the content:
      int pos_top = dparms->line_bottom + 2;

      int pos_left = dparms->margin_left + 1;
      // put cursor at top/left of top margin
      ti_printf("\x1b[%d;%dH", pos_top, pos_left);

      // print empty lines for each row
      for (int i=0; i<dparms->margin_bottom; ++i)
      {
         // Erase chars-count characters on this line
         ti_printf("\x1b[%dX", dparms->chars_count);
         // Down one to next line
         ti_printf("\x1b[1B");
         ti_printf("\x1b[%G", pos_left);
      }

      // Leave with cursor at top/left
      ti_printf("\x1b[%d;%dH", pos_top, pos_left);

      result = PWB_SUCCESS;
   }

   return result;
}


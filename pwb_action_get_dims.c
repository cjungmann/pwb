#include "pwb_actions.h"

#include <stdio.h>
#include <argeater.h>
#include <contools.h>

#include "pwb_keymap.h"
#include "pwb_argeater.h"
#include "pwb_errors.h"

void insert_dim(HASH_TABLE *target, const char *name, int value)
{
   char intbuffer[20];
   snprintf(intbuffer, sizeof(intbuffer), "%d", value);
   assoc_insert(target, savestring(name), intbuffer);
}


/**
 * @brief Get various dimensions
 *
 * This is an unusual action in that the handle name is optional.
 * Because of this, it comes here as a 'waived handle' action, with
 * @p ignored_handle set to NULL.
 *
 * If a handle is provided, this action returns several pwb-specific
 * dimensions.  If there is no handle, it only returns the screen
 * dimensions.  The no-handle option is useful for positioning a
 * message box in source script `pwb_show_message`.
 *
 * Another unusual feature of this action is that it creates an
 * associative array, PWB_ASSOC, if an existing one is not provided.
 * An error is generated if a -A variable is not an associative array.
 */
PWB_RESULT pwb_action_get_dims(PWBH *ignored_handle, ACLONE *args)
{
   PWB_RESULT result = PWB_FAILURE;

   PWBH *handle = NULL;

   // collect arguments
   const char *handle_name = NULL;
   const char *report_var_name = "PWB_ASSOC";

   AE_ITEM items[] = {
      { &handle_name, "handle", '\0', AET_ARGUMENT,
        "optional handle for more complete report" },

      { &report_var_name, "assoc", 'A', AET_VALUE_OPTION,
        "name of associative array for returning results" }
   };

   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      if (handle_name)
      {
         handle = pwb_get_handle_from_name(handle_name);
         if (!handle)
         {
            result = PWB_UNKNOWN_HANDLE;
            (*error_sink)("Unrecognized PWB handle name '%s'.", handle_name);
            goto early_exit;
         }
      }

      // Get the associative array
      SHELL_VAR *sv = NULL;
      HASH_TABLE *report_array = NULL;

      sv = find_variable(report_var_name);
      if (sv)
      {
         if (!assoc_p(sv))
            (*error_sink)("Variable %s is not an associative array", report_var_name);
         else
            report_array = assoc_cell(sv);
      }
      else
      {
         report_array = assoc_create(DEFAULT_HASH_BUCKETS);
         if (report_array)
         {
            sv = bind_variable(report_var_name, "", 0);
            pwb_dispose_variable_value(sv);
            var_setassoc(sv, report_array);
            VSETATTR(sv, att_assoc);
         }
      }

      if (report_array)
      {
         int lines, cols;
         get_screen_size(&lines, &cols);

         insert_dim(report_array, "screen_lines",  lines);
         insert_dim(report_array, "screen_cols",   cols);
         if (handle)
         {
            insert_dim(report_array, "data_rows",     handle->dparms.row_count);
            insert_dim(report_array, "pager_lines",   handle->dparms.line_count);
            insert_dim(report_array, "pager_cols",    handle->dparms.chars_count);
            insert_dim(report_array, "margin_top",    handle->dparms.margin_top);
            insert_dim(report_array, "margin_bottom", handle->dparms.margin_bottom);
            insert_dim(report_array, "margin_top",    handle->dparms.margin_top);
            insert_dim(report_array, "margin_left",   handle->dparms.margin_left);
            insert_dim(report_array, "margin_right",  handle->dparms.margin_right);
            insert_dim(report_array, "row_top",       handle->dparms.index_row_top);
            insert_dim(report_array, "row_focus",     handle->dparms.index_row_focus);
         }

         // If a supplied variable was uninitialized, it must be made
         // visible for the calling function to use it:
         if (invisible_p(sv))
            VUNSETATTR(sv, att_invisible);

         result = PWB_SUCCESS;
      }

   }

  early_exit:

   return result;
}

#include "pwb_actions.h"

#include <stdio.h>
#include <argeater.h>
#include <contools.h>

#include "pwb_keymap.h"
#include "pwb_errors.h"
#include "pwb_argeater.h"

void insert_dim(HASH_TABLE *target, const char *name, int value)
{
   char intbuffer[20];
   snprintf(intbuffer, sizeof(intbuffer), "%d", value);
   assoc_insert(target, savestring(name), intbuffer);
}


PWB_RESULT pwb_action_get_dims(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_FAILURE;

   // collect arguments
   static SHELL_VAR* report_var = NULL;

   static AE_ITEM items[] = {
      { (const char**)&report_var, "report", '\0', AET_ARGUMENT,
        "name of associate array to hold results", "INFO",
        pwb_argeater_assoc_ensurer}
   };

   AE_MAP map = INIT_MAP(items);
   if (!argeater_process(args, &map))
   {
      (*error_sink)("Error processing arguments.");
      result = PWB_FAILURE;
   }
   else
   {
      HASH_TABLE *report_array = assoc_cell(report_var);

      int lines, cols;
      get_screen_size(&lines, &cols);

      insert_dim(report_array, "data_rows",     handle->dparms.row_count);
      insert_dim(report_array, "screen_lines",  lines);
      insert_dim(report_array, "screen_cols",   cols);
      insert_dim(report_array, "pager_lines",   handle->dparms.line_count);
      insert_dim(report_array, "pager_cols",    handle->dparms.chars_count);
      insert_dim(report_array, "margin_top",    handle->dparms.margin_top);
      insert_dim(report_array, "margin_bottom", handle->dparms.margin_bottom);
      insert_dim(report_array, "margin_top",    handle->dparms.margin_top);
      insert_dim(report_array, "margin_left",   handle->dparms.margin_left);
      insert_dim(report_array, "margin_right",  handle->dparms.margin_right);
      insert_dim(report_array, "row_top",       handle->dparms.index_row_top);
      insert_dim(report_array, "row_focus",     handle->dparms.index_row_focus);

      // If a supplied variable was uninitialized, it must be made
      // visible for the calling function to use it:
      if (invisible_p(report_var))
         VUNSETATTR(report_var, att_invisible);

      result = PWB_SUCCESS;
   }

   return result;
}

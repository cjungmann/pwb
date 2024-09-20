#include "pwb_actions.h"

#include <stdio.h>
#include <argeater.h>
#include <contools.h>

#include "pwb_keymap.h"
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
   if (argeater_process(args, &map))
   {
      HASH_TABLE *report_array = assoc_cell(report_var);

      int rows, cols;
      get_screen_size(&rows, &cols);

      insert_dim(report_array, "screen_rows",   rows);
      insert_dim(report_array, "screen_cols",   cols);
      insert_dim(report_array, "margin_top",    handle->dparms.margin_top);
      insert_dim(report_array, "margin_bottom", handle->dparms.margin_bottom);
      insert_dim(report_array, "margin_top",    handle->dparms.margin_top);
      insert_dim(report_array, "margin_left",   handle->dparms.margin_left);
      insert_dim(report_array, "margin_right",  handle->dparms.margin_right);

      // If a supplied variable was uninitialized, it must be made
      // visible for the calling function to use it:
      if (invisible_p(report_var))
         VUNSETATTR(report_var, att_invisible);

      result = PWB_SUCCESS;
   }

   return result;
}

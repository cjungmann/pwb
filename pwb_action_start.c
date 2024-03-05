#include "pwb_actions.h"

#include <stdio.h>
#include <argeater.h>
#include <contools.h>

#include "pwb_keymap.h"
#include "pwb_argeater.h"

PWB_RESULT pwb_action_start(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_FAILURE;

   static unsigned int top_row = -1;
   static unsigned int focus_row = -1;
   static KCLASS base_keymap = { 0 };
   static KCLASS aux_keymap = { 0 };
   static const char *help_flag = NULL;

   // Extra step required to setup default keymap:
   result = initialize_kclass(&base_keymap, (*get_default_kdata)());

   static AE_ITEM items[] = {
      { &help_flag, "help", 'h', AET_FLAG_OPTION,
        "Display help" },

      { (const char**)&top_row, "top", 't', AET_VALUE_OPTION,
        "Source row of top screen line", "TOP_ROW", pwb_argeater_int_setter },

      { (const char**)&focus_row, "focus", 'f', AET_VALUE_OPTION,
        "Source row that has user focus", "FOCUS_ROW", pwb_argeater_int_setter },

      { (const char **)&base_keymap, "base", 'b', AET_VALUE_OPTION,
        "Base keymap, override default navigation keymap",
        "BASE_KEYMAP", argeater_kclass_setter },

      { (const char **)&aux_keymap, "aux", 'a', AET_VALUE_OPTION,
        "auxiliary keymap to handle keys not handled by the base keymap",
        "AUXILIARY_KEYMAP", argeater_kclass_setter }
   };

   static AE_MAP map = INIT_MAP(items);

   if (argeater_process(args, &map))
   {
      if (help_flag)
      {
         printf("Here's some help\n");
         result = PWB_SUCCESS;
      }
      else
      {
         pwbh_calc_borders(handle);

         ARV arv = ARV_REPLOT_DATA;
         while (arv != ARV_EXIT)
         {
            if (arv == ARV_REPLOT_DATA)
               pager_plot(&handle->dparms);

            const char *keys = get_keystroke(NULL, 0);
            arv = pwb_run_keystroke(handle, keys, &base_keymap, &aux_keymap);
         }
      }
   }

   return result;
}


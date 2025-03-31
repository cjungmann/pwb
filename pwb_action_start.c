#include "pwb_actions.h"
#include "pwb_errors.h"

#include <stdio.h>
#include <argeater.h>
#include <contools.h>
#include <assert.h>

#include "pwb_keymap.h"
#include "pwb_argeater.h"

PWB_RESULT pwb_action_start(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_FAILURE;

   unsigned int top_row = -1;
   unsigned int focus_row = -1;

   // Singleton, reuse when possible:
   KCLASS default_keymap = { 0 };

   KCLASS base_keymap = { 0 };
   KCLASS aux_keymap = { 0 };
   const char *help_flag = NULL;
   const char *exit_keystroke = NULL;

   // Initialize singleton (static) base_keymap only once:
   if (default_keymap.data == NULL)
   {
      // Don't disturb the 'result' default value of PWB_FAILURE
      // with a PWB_SUCCESS return of initialize_kclass:
      if (PWB_SUCCESS != initialize_kclass(&default_keymap, (*get_default_kdata)()))
      {
         (*error_sink)("Unexpected failure during global kclass initialization.");
         return PWB_FAILURE;
      }
   }

   AE_ITEM items[] = {
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
        "AUXILIARY_KEYMAP", argeater_kclass_setter },

      { &exit_keystroke, "exit_key" 'k', AET_VALUE_OPTION,
        "Name of variable in which to return the keystroke that ended the interaction",
        "EXIT_KEY" }
   };

   AE_MAP map = INIT_MAP(items);

   // We're assuming this, let's assert it, too:
   assert(result==PWB_FAILURE);

   if (argeater_process(args, &map))
   {
      // As of now, none of the functions called within the
      // block return a success/fail value, so we might as well
      // just indicate success right away.  We might have to
      // address hidden errors in the code later (ominous music).
      result = PWB_SUCCESS;

      if (help_flag)
         printf("Here's some help\n");
      else
      {
         pwbh_calc_borders(handle);

         ARV arv = ARV_REPLOT_DATA;

         // Sort out which keymaps to use here, outside of the while look
         KCLASS *basekm = NULL;
         KCLASS *auxkm = NULL;
         if ( base_keymap.data && *base_keymap.entry_count>0 )
            basekm = &base_keymap;
         else
            basekm = &default_keymap;

         if ( aux_keymap.data && *aux_keymap.entry_count > 0 )
            auxkm = &aux_keymap;

         while (arv != ARV_EXIT)
         {
            if (arv == ARV_REPLOT_DATA)
               pager_plot(&handle->dparms);

            const char *keys = get_keystroke(NULL, 0);
            if (keys)
               arv = pwb_run_keystroke(handle, keys, basekm, auxkm);
         }

         if ( exit_keystroke )
         {
            SHELL_VAR *sv = find_variable(exit_keystroke);
            if (!sv)
               sv = bind_variable(exit_keystroke, "", 0);

            if (sv)
            {
               pwb_dispose_variable_value(sv);
               sv->value = savestring(keys);
               if (invisible_p(sv))
                  VUNSETATTR(sv, att_invisible);
            }
         }
      }
   }

   return result;
}


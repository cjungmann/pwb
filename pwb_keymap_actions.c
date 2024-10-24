#include "pwb_builtin.h"
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

PWB_RESULT pwb_action_declare_keymap(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_FAILURE;
   static const char *keymap_handle_name = NULL;
   static const char *array_name = NULL;
   static AE_ITEM items[] = {
      { &keymap_handle_name, "name", '\0', AET_ARGUMENT,
        "name of new keymap handle" },

      { &array_name, "array_name", '\0', AET_ARGUMENT,
        "array containing keymap values", NULL,
        argeater_array_name_setter }
   };
   static AE_MAP map = INIT_MAP(items);
   if (!argeater_process(args, &map))
   {
      (*error_sink)("Error processing arguments.");
      result = PWB_FAILURE;
   }
   else
   {
      if (keymap_handle_name && array_name)
         result = pwb_make_kdata_shell_var(keymap_handle_name, array_name, KEYMAP_LABEL);
   }

   return result;
}



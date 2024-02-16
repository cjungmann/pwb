#include "pwb_actions.h"
#include <argeater.h>
#include <contools.h>
#include <stdio.h>
#include <stdlib.h>    // for malloc/free
#include <alloca.h>

#include "pwb_handle.h"
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

PWB_RESULT pwb_action_declare(PWBH *handle, ACLONE *args)
{
   PWB_RESULT result = PWB_FAILURE;

   static const char *handle_name = NULL;
   static const char *data_source = NULL;
   static int data_count = -1;
   static const char *print_func = NULL;
   static const char *exec_func = NULL;
   static const char *extra_data = NULL;
   static AE_ITEM items[] = {
      { &handle_name, "handle_name", '\0', AET_ARGUMENT,
      "name of new handle" },
      { &data_source, "data_source", '\0', AET_ARGUMENT,
        "name of data source" },
      { (const char **)&data_count, "data_count", '\0', AET_ARGUMENT,
        "lines in data source", NULL, argeater_int_setter },
      { &print_func, "printer", '\0', AET_ARGUMENT,
        "name of line print function" },
      { &exec_func, "exec", 'e', AET_VALUE_OPTION,
        "name of line execute function" },
      { &extra_data, "extra_data", 'd', AET_VALUE_OPTION,
        "name of extra data variable" }
   };

   AE_MAP map = INIT_MAP(items);
   if (argeater_process(args, &map))
   {
      int handle_size = pwb_calc_handle_size(data_source,
                                             print_func,
                                             handle_name,
                                             exec_func,
                                             extra_data);
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
                                               extra_data);

         WORD_LIST *wl_ptr = pwbh->printer_wl;
         __attribute__((unused))
         WORD_LIST *wl_length = &wl_ptr[3];

         SHELL_VAR *sv = bind_variable(handle_name, "", 0);
         if (sv)
         {
            pwb_dispose_variable_value(sv);
            sv->value = buff;
            sv->attributes |= att_special;
            result = PWB_SUCCESS;

            argeater_dump_actions(&map);
         }
      }
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

PWB_RESULT pwb_action_start(PWBH *handle, ACLONE *args)
{
   pwb_terminal_init();
   pager_init();

   pwbh_calc_borders(handle);

   PWB_RESULT result = PWB_FAILURE;
   KCLASS kclass_default;
   // result = (get_default_keymap(&kclass_default, "keymap default", alloca, NULL));
   result = (get_default_keymap(&kclass_default, "keymap default", malloc, free));
   if (result == PWB_SUCCESS)
   {
      DPARMS *dparms = &handle->dparms;
      ARV arv = ARV_REPLOT_DATA;
      while (arv != ARV_EXIT)
      {
         if (arv == ARV_REPLOT_DATA)
            pager_plot(dparms);

         const char *keys = get_keystroke(NULL, 0);
         arv = pwb_run_keystroke(handle, keys, &kclass_default);
      }

      free(kclass_default.data);
   }

   return result;
}


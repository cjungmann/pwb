#include "pwb_builtin.h"
#include "pwb_actions.h"
#include <argeater.h>
#include <pager.h>
#include "pwb_handle.h"

#include <stdio.h>

const char *help_flag = NULL;
const char *data_source = NULL;
const char *printer_function = NULL;
const char *exec_function = NULL;
const char *data_extra = NULL;

AE_ITEM actions[] = {
   { &help_flag, "help", 'h', AET_FLAG_OPTION,
     "Show usage instructions", NULL,
     argeater_string_setter },

   { &data_source, "data_source", 'd', AET_VALUE_OPTION,
     "Name of a Bash data source", "DATA",
     argeater_string_setter },

   { &printer_function, "printer_function", 'p', AET_VALUE_OPTION,
     "Name of a printer function", "PRINTER",
     argeater_string_setter },

   { &exec_function, "exec_function", 'e', AET_VALUE_OPTION,
     "(Optional) execution function", "EXEC_FUNCTION",
     argeater_string_setter },

   { &data_extra, "data_extra", 'x', AET_VALUE_OPTION,
     "(Optional) name of Bash variable to pass to print and exec functions",
     "EXTRA_DATA",
     argeater_string_setter }
};

void show_usage(AE_MAP *map)
{
   argeater_show_usage(map,"pwb");
   argeater_show_options(map, 3);
   argeater_show_arguments(map, 3);
}


static int pwb_builtin(WORD_LIST *list)
{
   int exit_code = EXECUTION_FAILURE;

   PWB_RESULT result = perform_verb(list);
   if (result == PWB_SUCCESS)
      exit_code = EXECUTION_SUCCESS;

   return exit_code;
}

static char *desc_pwb[] = {
   "pwb - Pager with benefits",
   "",
   (char*)NULL
};

struct builtin pwb_struct = {
   .name      = "pwb",
   .function  = pwb_builtin,
   .flags     = BUILTIN_ENABLED,
   .long_doc  = desc_pwb,
   .short_doc = "pwb file_name [OPTIONS]",
   .handle    = 0
};

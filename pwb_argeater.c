#include "pwb_handle.h"
#include "pwb_argeater.h"
#include <argeater.h>
#include <stdio.h>

bool argeater_pwbh_setter(const char **target, const char *value)
{
   SHELL_VAR *sv = find_variable(value);
   if (sv)
   {
      if (pwbh_p(sv))
      {
         *(PWBH**)target = pwbh_cell(sv);
         return true;
      }
      else
         fprintf(stderr, "Variable '%s' is not a pwb handle.", value);
   }
   else
      fprintf(stderr, "Failed to find handle variable '%s'.", value);

   return false;
}

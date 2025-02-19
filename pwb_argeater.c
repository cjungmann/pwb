#include "pwb_handle.h"
#include "pwb_argeater.h"
#include "pwb_errors.h"
#include <argeater.h>
#include <stdio.h>
#include <stdlib.h>   // for strtol()

/**
 * @brief Reimplement int_setter to allow using our error reporting
 */
bool pwb_argeater_int_setter(const char **target, const char *value)
{
   if (*value)
   {
      char *end;
      long val = strtol(value, &end, 10);
      if (*end == '\0')
      {
         *(int*)target = (int)val;
         return true;
      }
      else
         (*error_sink)("Value '%s' does not convert to an integer.", value);
   }
   else
      (*error_sink)("Integer argument was empty.");

   return false;
}

/**
 * @brief Reimplement int_setter to allow using our error reporting
 */
bool pwb_argeater_unsigned_int_setter(const char **target, const char *value)
{
   if (*value)
   {
      char *end;
      unsigned long val = strtoul(value, &end, 10);
      if (*end == '\0')
      {
         *(unsigned int*)target = (unsigned int)val;
         return true;
      }
      else
         (*error_sink)("Value '%s' does not convert to an unsigned integer.", value);
   }
   else
      (*error_sink)("Unsigned integer argument was empty.");

   return false;
}

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
         (*error_sink)("Variable '%s' is not a pwb handle.", value);
   }
   else
      (*error_sink)("Failed to find handle variable '%s'.", value);

   return false;
}

bool argeater_array_name_setter(const char **target, const char *value)
{
   SHELL_VAR *sv = find_variable(value);
   if (sv)
   {
      if (array_p(sv))
      {
         *target = value;
         return true;
      }
      else
         (*error_sink)("Variable '%s' is not an array.", value);
   }
   else
      (*error_sink)("Failed to find variable '%s'.", value);
   return false;
}

bool argeater_array_setter(const char **target, const char *value)
{
   SHELL_VAR *sv = find_variable(value);
   if (sv)
   {
      if (array_p(sv))
      {
         *(ARRAY**)target = array_cell(sv);
         return true;
      }
      else
         (*error_sink)("Variable '%s' is not a shell array.", value);
   }
   else
      (*error_sink)("Failed to find handle variable '%s'.", value);

   return false;
}

bool pwb_argeater_function_setter(const char **target, const char *value)
{
   SHELL_VAR *sv = find_function(value);
   if (sv)
   {
      if (function_p(sv))
      {
         *target = value;
         return true;
      }
      else
         (*error_sink)("Shell variable '%s' is not a function.\n", value);
   }
   else
      (*error_sink)("Failed to find script function '%s'.\n", value);

   return false;
}

/**
 * @brief Ensures named associative array variable exists
 *
 * If the variable exists and is an associative array, it is
 * left alone.  Otherwise, The variable is created if it doesn't
 * already exist.  If the named variable doesn't exist, an
 * associative array of the given name is created.
 */
bool pwb_argeater_assoc_ensurer(const char **target, const char *value)
{
   SHELL_VAR *sv = find_variable(value);
   if (sv)
   {
      // Clear array if proper type, otherwise dispose of it
      if (assoc_p(sv))
         assoc_flush(assoc_cell(sv));
      else
      {
         dispose_variable(sv);
         sv = NULL;
      }
   }

   if (!sv)
   {
      if (variable_context == 0)
         sv = make_new_assoc_variable((char*)value);
      else
         sv = make_local_assoc_variable((char*)value, variable_context);
   }

   if (sv)
   {
      *target = *(char**)&sv;
      return true;
   }

   (*error_sink)("Failed to secure associative array %s", value);
   return false;
}


bool argeater_kclass_setter(const char **target, const char *value)
{
   SHELL_VAR *sv = find_variable(value);
   if (sv)
   {
      if (specialvar_p(sv))
      {
         int diff = strcmp(sv->value, KEYMAP_LABEL);
         ++diff;
      }
      if (keymap_p(sv))
      {
         KCLASS *kclass = (KCLASS*)target;
         KDATA *kdata = (KDATA*)sv->value;
         PWB_RESULT result = initialize_kclass(kclass, kdata);
         return result == PWB_SUCCESS;
      }
      else
         (*error_sink)("Shell variable '%s' is not a keymap.\n", value);
   }
   else
      (*error_sink)("Failed to find keymap variable '%s'.\n", value);

   return false;
}


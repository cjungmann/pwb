#include "pwb_builtin.h"
#include "pwb_handle.h"
#include "pwb_errors.h"

#include <stdarg.h>
#include <stdio.h>

type_error_sink error_sink = pwb_error_shell_var;

void pwb_error_shell_var(const char *format, ...)
{
   va_list args;
   va_start(args, format);
   int len = vsnprintf(NULL, 0, format, args);
   va_end(args);

   if (len > 0)
   {
      ++len;
      char *buffer = xmalloc(len);
      if (buffer)
      {
         va_start(args, format);
         vsnprintf(buffer, len, format, args);
         va_end(args);

         SHELL_VAR *sv = find_variable(ERROR_NAME);
         if (!sv)
            sv = bind_variable(ERROR_NAME, "", 0);

         if (sv)
         {
            pwb_dispose_variable_value(sv);
            sv->value = buffer;
         }
      }
   }
}

void pwb_error_print(const char *format, ...)
{
  va_list args;
   va_start(args, format);
   fprintf(stderr, format, args);
   va_end(args);
}

void pwb_error_clear(void)
{
   SHELL_VAR *sv = find_variable(ERROR_NAME);
   if (!sv)
      sv = bind_variable(ERROR_NAME, "", 0);

   if (sv)
      pwb_dispose_variable_value(sv);
}

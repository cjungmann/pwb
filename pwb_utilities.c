#include "pwb_utilities.h"
#include <string.h>
#include <math.h>    // for log10
#include <stdio.h>   // for snprintf

/**
 * @brief Returns memory required to save a string, 0 if NULL
 *
 * Function checks for NULL (returns 0 length in that case),
 * and includes room for a terminating \0.
 */
int get_string_saved_len(const char *str)
{
   int slen = 0;
   if (str && (slen = strlen(str))>0)
      ++slen;
   return slen;
}

/**
 * @brief Packs a string with other strings into a block of memory.
 *
 * Tracks progress packing block through pointer-to-pointer arguments.
 * No copy is made if the string is NULL or of zero-length.  @p string
 * will be set to NULL.
 *
 * @param "string"       pointer to copy of @p value in the block
 * @param "buff_pointer" pointer to next available address in the block
 * @param "buff_limit"   pointer to end of block to prevent overruns
 * @param "value"        string value to copy into the block
 *
 * @return true for success, false for failure due to insufficient memory
 */
bool pack_string_in_block(const char **string,
                          char **buff_pointer,
                          char *buff_limit,
                          const char *value)
{
   *string = NULL;
   int slen;
   if (value && (slen = strlen(value)) > 0)
   {
      // check for enough space
      if (buff_limit - *buff_pointer <= slen)
         return false;

      *string = *buff_pointer;
      memcpy(*buff_pointer, value, slen);
      (*buff_pointer) += slen;
      **buff_pointer = '\0';
      (*buff_pointer)++;
   }

   return true;
}

bool add_var_attribute(char **ptr, char *ptr_end, int *byte_count, const char *value)
{
   bool retval = true;

   int temp_len = strlen(value);

   if (*ptr)
   {
      char *lptr= *ptr;

      if (lptr + temp_len < ptr_end)
      {
         // Only add space IFS if not first attribute
         if (*byte_count > 0)
            *lptr++ = ' ';

         memcpy(lptr, value, temp_len);
         lptr += temp_len;

         // Update source pointer
         *ptr = lptr;
      }
      else
         retval = false;
   }

   *byte_count += temp_len + ( (*byte_count > 0) ? 1 : 0 );

   return retval;
}

int get_var_parameters(char *buffer,
                       int bufflen,
                       SHELL_VAR *sv,
                       bool include_context,
                       bool include_attributes)
{
   int bytes_tally = 0;

   char *ptr = buffer;
   char *ptr_end = ptr + bufflen;

   if (sv)
   {
      const char *attr_str = "generic";
      if (array_p(sv))
         attr_str = "array";
      else if (integer_p(sv))
         attr_str = "integer";
      else if (assoc_p(sv))
         attr_str = "associative";
      else if (function_p(sv))
         attr_str = "function";

      if (! add_var_attribute(&ptr, ptr_end, &bytes_tally, attr_str))
         goto out_of_memory;

      if (include_context)
      {
         if (ptr && ptr < ptr_end)
         {
            // leave room for terminating '\0'.  It won't be counted,
            // it will be later overwriten, but it allows a single
            // digit to be printed
            int pcount = snprintf(ptr, 1+ptr_end-ptr, " %d", sv->context);
            ptr += pcount;
            bytes_tally += pcount;
         }
         else
            bytes_tally += 1 + ( sv->context==0?1:(floor(log10(sv->context))+1) );
      }

      struct ATT_TABLE { int flag; const char *label; };

      // The 'att_' prefix flags are found in Bash source file variables.h
      // att_table and att_count won't change after loading:
      const static struct ATT_TABLE att_table[] = {
         { att_exported,  "export"    },
         { att_readonly,  "readonly"  },
         { att_local,     "local"     },
         { att_trace,     "trace"     },
         { att_uppercase, "uppercase" },
         { att_lowercase, "lowercase" },
         { att_nameref,   "nameref"   },
         { att_invisible, "invisible" },
         { att_imported,  "imported"  },
         { att_special,   "special"   }
      };
      static int att_count = sizeof(att_table) / sizeof(struct ATT_TABLE);

      if (include_attributes)
      {
         const struct ATT_TABLE *tptr = att_table;
         const struct ATT_TABLE *tend = tptr + att_count;
         while ( tptr < tend )
         {
            if (sv->attributes & tptr->flag)
               if (! add_var_attribute(&ptr, ptr_end, &bytes_tally, tptr->label ))
                  goto out_of_memory;
            ++tptr;
         }
      }
   }

  out_of_memory:
   if (ptr && ptr < ptr_end)
      *ptr = '\0';

   return bytes_tally + 1;
}

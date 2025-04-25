#include "pwb_utilities.h"
#include <string.h>

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

int get_var_attributes(char *buffer, int bufflen, SHELL_VAR *sv)
{
   int bytes_tally = 0;

   char *ptr = buffer;
   // Reserve end-type for a '\0'
   char *ptr_end = ptr + bufflen - 1;

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

      if (exported_p(sv))
         if (! add_var_attribute(&ptr, ptr_end, &bytes_tally, "exported"))
            goto out_of_memory;

      if (local_p(sv))
         if (! add_var_attribute(&ptr, ptr_end, &bytes_tally, "local"))
            goto out_of_memory;

      if (readonly_p(sv))
         if (! add_var_attribute(&ptr, ptr_end, &bytes_tally, "readonly"))
            goto out_of_memory;

      if (uppercase_p(sv))
         if (! add_var_attribute(&ptr, ptr_end, &bytes_tally, "uppercase"))
            goto out_of_memory;

      if (lowercase_p(sv))
         if (! add_var_attribute(&ptr, ptr_end, &bytes_tally, "lowercase"))
            goto out_of_memory;

      if (capcase_p(sv))
         if (! add_var_attribute(&ptr, ptr_end, &bytes_tally, "capcase"))
            goto out_of_memory;

      if (nameref_p(sv))
         if (! add_var_attribute(&ptr, ptr_end, &bytes_tally, "nameref"))
            goto out_of_memory;

      if (trace_p(sv))
         if (! add_var_attribute(&ptr, ptr_end, &bytes_tally, "trace"))
            goto out_of_memory;

      if (invisible_p(sv))
         if (! add_var_attribute(&ptr, ptr_end, &bytes_tally, "invisible"))
            goto out_of_memory;

      if (specialvar_p(sv))
         if (! add_var_attribute(&ptr, ptr_end, &bytes_tally, "special"))
            goto out_of_memory;
   }

  out_of_memory:
   if (ptr && ptr <= ptr_end)
      *ptr = '\0';

   return bytes_tally + 1;
}

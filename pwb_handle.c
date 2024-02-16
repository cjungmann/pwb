#include "pwb_builtin.h"

#include "pwb_handle.h"
#include <string.h>
#include <assert.h>


/**
 * @brief Prepare block of memory to be linked WORD_LIST and WORD_DESC elements
 * @param "buffer"   buffer of sufficient size for number of @p els.
 * @param "els"      number of WORD_LIST->WORD_DESC elements to initialize
 *
 * Calculate @p buffer size by multiplying `(sizeof(WORD_LIST) + sizeof(WORD_DESC))`
 * by @p els.
 *
 * The memory will be organized as an array of @p els WORD_LIST,
 * followed by an array of @p els WORD_DESC, with all the elements
 * also connected as linked elements.
 *
 * Elements in the returned WORD_LIST can thus be accessed directly
 * through indexed access or by walking the links.
 */
WORD_LIST * pwb_initialize_word_list(char* buffer, int els)
{
   WORD_LIST *retval = (WORD_LIST*)buffer;
   WORD_LIST *wlptr = retval;
   WORD_LIST *wlend = wlptr + els;
   WORD_DESC *wdptr = (WORD_DESC*)wlend;

   while (wlptr < wlend)
   {
      WORD_LIST *wlnext = wlptr+1;
      if (wlnext >= wlend)
         wlnext = NULL;

      wlptr->next = wlnext;
      wlptr->word = wdptr;

      // Leave WORD_DESC set to NULL/0

      ++wlptr;
      ++wdptr;
   }

   return retval;
}

/**
 * @brief Calculate memory size required to be apportioned to a PWBH struct
 * @param "data_source_name"  name of Bash variable to pass to script
 *                            for data
 * @param "printer_func_name" name of Bash script function that will
 *                            print a text line
 * @param "exec_func_name"    optional name of Bash script function to
 *                            call to perform some action
 * @param "data_extra_name"   optional name of Bash variable that will
 *                            be passed to print and exec functions
 *                            for context
 *
 * Returns amount of memory needed to contain the entirety of a
 * PWBH and its contents.  Call this function to determine the
 * buffer size needed to pass to @ref pwb_initialize_handle.
 *
 * The function names are not important in this step because they
 * will be saved as pointers and not copied. They are here to be a
 * mirror of the arguments of @ref pwb_initialize_handle.
 *
 * The significant argument is @p exec_func_name.  If it is provided,
 * a WORD_LIST of arguments will be set aside for calling the exec
 * function.
 */
int pwb_calc_handle_size(const char *data_source_name,
                         const char *printer_func_name,
                         const char *handle_name,
                         const char *exec_func_name,
                         const char *data_extra_name)
{
   int total_bytes = sizeof(PWBH);
   int int_count = 0;
   int slen;

   total_bytes += 1 + strlen(data_source_name);

   // Reserve memory for strings
   if (data_source_name && (slen=strlen(data_source_name))>0)
      total_bytes += 1 + slen;
   if (data_extra_name && (slen=strlen(data_extra_name))>0)
      total_bytes += 1 + slen;
   if (handle_name && (slen=strlen(handle_name))>0)
      total_bytes += 1 + slen;

   if (printer_func_name && (slen=strlen(printer_func_name))>0)
   {
      total_bytes += 1 + slen;
      int_count += 3;
      total_bytes += pwb_calc_word_list_size(7,3);
   }
   if (exec_func_name && (slen=strlen(exec_func_name))>0)
   {
      total_bytes += 1 + slen;
      int_count += 1;
      total_bytes += pwb_calc_word_list_size(6,1);
   }

   return total_bytes + (int_count * WORD_LIST_INT_SIZE);
}

/**
 * @brief Allocate memory in @p buffer
 *
 * @param "data_source_name"  name of Bash variable to pass to script
 *                            for data
 * @param "printer_func_name" name of Bash script function that will
 *                            print a text line
 * @param "exec_func_name"    optional name of Bash script function to
 *                            call to perform some action
 * @param "data_extra_name"   optional name of Bash variable that will
 *                            be passed to print and exec functions
 *                            for context
 *
 * In the context of the application, these values will be taken from
 * command line arguments whose scope exceeds the scope of the handle,
 * so I'm just assigning the argument pointers to the appropriate
 * places in the PWBH.
 */
PWBH * pwb_initialize_handle(char *buffer,
                             int buffer_len,
                             const char *data_source_name,
                             int data_count,
                             const char *printer_name,
                             const char *handle_name,
                             const char *exec_name,
                             const char *data_extra_name)
{
#define MEMTEST free < buffer + buffer_len + 1
   memset(buffer, 0, buffer_len);

   PWBH *pwbh = (PWBH*)buffer;
   memcpy(pwbh, PWB_ID_STRING, sizeof(PWB_ID_STRING));

   // DPARMS basics:
   pwbh->dparms.row_count = data_count;
   pwbh->dparms.printer = pwb_line_printer;
   pwbh->dparms.data_extra = (void*)pwbh;

   // Allocate memory starting from just after the PWBH
   char *free = buffer + sizeof(PWBH);
   assert(MEMTEST);

   const static char *empty_string = "";
   const char *copy_data_source_name = empty_string;
   const char *copy_data_extra_name = empty_string;
   const char *copy_handle_name = empty_string;

   // Copy strings and assign copies to DPARMS members:
   // Copy and assign data_source_name
   int slen = strlen(data_source_name);
   memcpy(free, data_source_name, slen);
   free[slen] = '\0';
   copy_data_source_name = (char*)free;
   pwbh->dparms.data_source = (void*)free;
   free += slen + 1;

   if (handle_name != NULL && (slen = strlen(handle_name))>0)
   {
      memcpy(free, handle_name, slen);
      free[slen] = '\0';
      copy_handle_name = free;
      free += slen + 1;
   }

   // If provided, copy and assign data_extra_name:
   if (data_extra_name != NULL && (slen = strlen(data_extra_name))>0)
   {
      copy_data_extra_name = (char*)free;
      /*
       * DON'T set/change dparms.data_extra, the name of the extra data.
       * If it's included, will be in the parameter WORD_LISTs used for
       * calling the line-print function and the exec function.
       *
       * dparms.data_extra is cast to a DWBH pointer in the C code
       * line-print and exec functions, then the data from the DWBH is
       * used to call the Bash shell functions
       */
      // pwbh->dparms.data_extra = (void*)free;
      memcpy(free, data_extra_name, slen);
      free[slen] = '\0';
      free += slen + 1;
   }

   if (printer_name && (slen = strlen(printer_name))>0)
   {
      pwbh->printer_wl = pwb_initialize_word_list(free, 7);
      free += pwb_calc_word_list_size(7,3);
      assert(MEMTEST);

      // Copy and assign printer_name
      if (printer_name && (slen = strlen(printer_name))>0)
      {
         pwb_set_word_list_string_arg(pwbh->printer_wl, 0, free);
         memcpy(free, printer_name, slen);
         free[slen] = '\0';
         free += slen + 1;
      }
      else
         pwb_set_word_list_string_arg(pwbh->printer_wl, 0, empty_string);

      // $1 (int), print function row_index
      pwb_initialize_word_list_int_arg(pwbh->printer_wl, 1, free);
      free += WORD_LIST_INT_SIZE;
      assert(MEMTEST);

      // $2 (string), data source name
      pwb_set_word_list_string_arg(pwbh->printer_wl, 2, copy_data_source_name);

      // $3 (int), focus flag
      pwb_initialize_word_list_int_arg(pwbh->printer_wl, 3, free);
      free += WORD_LIST_INT_SIZE;
      assert(MEMTEST);

      // $4 (int), char limit
      pwb_initialize_word_list_int_arg(pwbh->printer_wl, 4, free);
      free += WORD_LIST_INT_SIZE;
      assert(MEMTEST);

      // $5 (string) handle name
      pwb_set_word_list_string_arg(pwbh->printer_wl, 5, copy_handle_name);

      // $6, Set or disconnect final WORD_LIST element based on data_extra provision
      if (copy_data_extra_name == empty_string)
         pwb_truncate_word_list_at_index(pwbh->printer_wl, 5);
      else
         pwb_set_word_list_string_arg(pwbh->printer_wl, 6, copy_data_extra_name);
   }

   // Process the exec word_list only if an exec function is provided:
   if (exec_name && (slen = strlen(exec_name))>0)
   {
      pwbh->exec_wl = pwb_initialize_word_list(free, 6);
      free += pwb_calc_word_list_size(5,1);
      assert(MEMTEST);

      // $0, Copy and assign exec_name
      pwb_set_word_list_string_arg(pwbh->exec_wl, 0, free);
      memcpy(free, exec_name, slen);
      free[slen] = '\0';
      free += slen + 1;

      // $1 (string) keystroke, unset because it will be set at each call

      // $2 (int) data row number
      pwb_initialize_word_list_int_arg(pwbh->exec_wl, 2, free);
      free += WORD_LIST_INT_SIZE;
      assert(MEMTEST);

      // $3 (string) data source name
      pwb_set_word_list_string_arg(pwbh->exec_wl, 3, copy_data_source_name);

      // $4 (string) handle name
      pwb_set_word_list_string_arg(pwbh->exec_wl, 4, copy_handle_name);

      // $5 (string) Set or disconnect final WORD_LIST element based on data_extra provision
      if (copy_data_extra_name == empty_string)
         pwb_truncate_word_list_at_index(pwbh->exec_wl, 4);
      else
         pwb_set_word_list_string_arg(pwbh->exec_wl, 5, copy_data_extra_name);
   }

   return pwbh;
}

PWBH *pwb_get_handle_from_name(const char *handle_name)
{
   assert(handle_name && strlen(handle_name)>0);
   SHELL_VAR *sv = find_variable(handle_name);
   if (sv && pwbh_p(sv))
      return pwbh_cell(sv);

   return NULL;
}


#ifdef PWB_HANDLE_MAIN

#include "pwb_builtin.h"
#include <alloca.h>
#include "pwbh_support.c"

void dump_word_list(WORD_LIST *wl)
{
   int count = 0;
   while (wl)
   {
      printf("%2d: '%s'\n", ++count, wl->word->word);
      wl = wl->next;
   }
}

int main(int argc, const char **argv)
{
   const char *ds_name = "dsource";
   int ds_count = 10;
   const char *pf_name = "printer_func";
   const char *ef_name = "exec_func";
   const char *de_name = "data_extra";

   int hsize = pwb_calc_handle_size(ds_name, pf_name, ef_name, de_name);
   char *buff = alloca(hsize);
   PWBH *pwbh = pwb_initialize_handle(buff, hsize, ds_name,
                                      ds_count, pf_name,
                                      ef_name, de_name);

   printf("The handle shows string '%s'\n", (char*)pwbh);

   printf("\nLet's set some strings:\n");

   pwbh_print_set_row_index(pwbh, 10);
   pwbh_print_set_focus(pwbh, 10);
   pwbh_print_set_length(pwbh, 50);

   pwbh_exec_set_row_number(pwbh, 15);
   pwbh_exec_set_keystroke(pwbh, "weird_keystroke, baby");

   printf("Dumping the printer function arguments:\n");
   dump_word_list(pwbh->printer_wl);

   printf("\nDumping the exec function arguments:\n");
   dump_word_list(pwbh->exec_wl);

   return 0;
}


#endif


/* Local Variables:                      */
/* compile-command: "gcc                  \*/
/*    -DPWB_HANDLE_MAIN                 \*/
/*    -std=c99 -Wall -Werror -ggdb        \*/
/*    -I/usr/include/bash                 \*/
/*    -I/usr/include/bash/include         \*/
/*    -fsanitize=address                  \*/
/*    -o test_pwb_handle pwb_handle.c \*/
/*    -lpager -ltinfo  " */
/* End: */

#include "pwb_builtin.h"

#include "pwb_handle.h"
#include "pwb_utilities.h"
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

   total_bytes += 1 + strlen(data_source_name);

   // Reserve memory for strings
   total_bytes += get_string_saved_len(data_source_name);
   total_bytes += get_string_saved_len(data_extra_name);
   total_bytes += get_string_saved_len(handle_name);
   total_bytes += get_string_saved_len(printer_func_name);
   total_bytes += get_string_saved_len(exec_func_name);

   // Reserve memory for WORD_LISTs for callback functions
   if (printer_func_name && *printer_func_name)
   {
      int_count += 3;
      total_bytes += pwb_calc_word_list_size(7,3);
   }
   if (exec_func_name && *exec_func_name)
   {
      total_bytes += pwb_calc_word_list_size(6,1);
      int_count += 1;
   }

   // Reserve memory for strings
   // int slen;
   // if (data_source_name && (slen=strlen(data_source_name))>0)
   //    total_bytes += 1 + slen;
   // if (data_extra_name && (slen=strlen(data_extra_name))>0)
   //    total_bytes += 1 + slen;
   // if (handle_name && (slen=strlen(handle_name))>0)
   //    total_bytes += 1 + slen;

   // if (printer_func_name && (slen=strlen(printer_func_name))>0)
   // {
   //    total_bytes += 1 + slen;
   //    int_count += 3;
   //    total_bytes += pwb_calc_word_list_size(7,3);
   // }
   // if (exec_func_name && (slen=strlen(exec_func_name))>0)
   // {
   //    total_bytes += 1 + slen;
   //    int_count += 1;
   //    total_bytes += pwb_calc_word_list_size(6,1);
   // }

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
                             const char *data_extra_name
                             // ,const char *head_printer_name
                             // ,const char *foot_printer_name
   )
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
   char *buff_end = buffer + buffer_len;

   // For PWB handle members
   // pwb_save_string_to_handle(&pwbh->head_printer_name,
   //                           &free, buff_end, head_printer_name);
   // pwb_save_string_to_handle(&pwbh->foot_printer_name,
   //                           &free, buff_end, foot_printer_name);

   // For DPARMS in header
   const char *copy_data_source_name = NULL;
   pack_string_in_block(&copy_data_source_name, &free, buff_end, data_source_name);
   pwbh->dparms.data_source = (char*)copy_data_source_name;

   // For WORD_LIST parameters
   const char *copy_printer_name = NULL;
   const char *copy_handle_name = NULL;
   const char *copy_exec_name = NULL;
   const char *copy_data_extra_name = NULL;

   pack_string_in_block(&copy_printer_name, &free, buff_end, printer_name);
   pack_string_in_block(&copy_handle_name, &free, buff_end, handle_name);
   pack_string_in_block(&copy_exec_name, &free, buff_end, exec_name);
   pack_string_in_block(&copy_data_extra_name, &free, buff_end, data_extra_name);

   // Prepare WORD_LIST for printer function callback
   if (copy_printer_name)
   {
      pwbh->printer_wl = pwb_initialize_word_list(free, 7);
      free += pwb_calc_word_list_size(7,3);

      // Set integer WORD_LIST elements first
      // row_index
      pwb_initialize_word_list_int_arg(pwbh->printer_wl, 1, free);
      free += WORD_LIST_INT_SIZE;
      // focus_flag
      pwb_initialize_word_list_int_arg(pwbh->printer_wl, 3, free);
      free += WORD_LIST_INT_SIZE;
      // char_limit
      pwb_initialize_word_list_int_arg(pwbh->printer_wl, 4, free);
      free += WORD_LIST_INT_SIZE;

      // Confirm we're on track, memory-wise
      assert(MEMTEST);

      // Set string WORD_LIST elements
      pwb_set_word_list_string_arg(pwbh->printer_wl, 0, copy_printer_name);
      pwb_set_word_list_string_arg(pwbh->printer_wl, 2, copy_data_source_name);
      pwb_set_word_list_string_arg(pwbh->printer_wl, 5, copy_handle_name);

      // Set last WORD_LIST item according to supplied arguments
      if (copy_data_extra_name)
         pwb_set_word_list_string_arg(pwbh->printer_wl, 6, copy_data_extra_name);
      else
         pwb_truncate_word_list_at_index(pwbh->printer_wl, 5);
   }

   if (copy_exec_name)
   {
      pwbh->exec_wl = pwb_initialize_word_list(free, 6);
      free += pwb_calc_word_list_size(6, 1);

      // Set integer WORD_LIST elements first
      // data row number
      pwb_initialize_word_list_int_arg(pwbh->exec_wl, 2, free);
      free += WORD_LIST_INT_SIZE;

      pwb_set_word_list_string_arg(pwbh->exec_wl, 0, exec_name);
      // Skip WORD_LIST index 1 because it will be allocated for each function call
      pwb_set_word_list_string_arg(pwbh->exec_wl, 3, copy_data_source_name);
      pwb_set_word_list_string_arg(pwbh->exec_wl, 4, copy_handle_name);

      // Set last WORD_LIST item according to supplied arguments
      if (copy_data_extra_name)
         pwb_set_word_list_string_arg(pwbh->exec_wl, 5, copy_data_extra_name);
      else
         pwb_truncate_word_list_at_index(pwbh->exec_wl, 4);
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

#include <stdio.h>
#include <string.h>       // strlen()

#include <unistd.h>       // write()

#include <stdarg.h>       // for va_list, etc
#include <alloca.h>

#include <assert.h>
#include <sys/ioctl.h>    // for ioctl() in get_screen_size()

#include "pager.h"
#include "termstuff.h"
#include "../lib_freader/freader.h"
#include "../lib_cons/cons.h"


/**
 * @brief Implementation of line printer function to be used in a DPARMs struct
 *
 * @param "row_index"   index of row in source data of the indicated line
 * @param "indicated"   flag, highlight if *true*, normal if *false*
 * @param "length"      number of characters to print (include spaces to fill line)
 * @param "data_source" to be recase as appropriate data source from which to get data
 * @return Number of characters printed, kind of meaningless.
 */
int lindex_print(int row_index, int indicated, int length, void *data_source)
{
   LINDEX *lindex = (LINDEX*)data_source;
   const char *str = get_lindex_line(lindex, row_index);
   if (indicated)
      ti_write_str("\x1b[7m");

   char *buff = (char*)alloca(length+1);
   snprintf(buff, length+1, "%-*s", length, str);
   ti_write_str(buff);
   // ti_printf("%-*s", length, str);

   if (indicated)
      ti_write_str("\x1b[27m");

   return length;
}

typedef struct termcap_entry_action {
   TCENTRY entry;
   PACTION action;
} TCE_ACTION;

TCE_ACTION keys[] = {
   { { "", "q" }, pager_quit },
   { {"kcud1"}, pager_focus_down_one },   // key down
   { {"kcuu1"}, pager_focus_up_one },     // key up
   { {"knp"},   pager_focus_down_page },  // key next page (page down)
   { {"kpp"},   pager_focus_up_page  },   // key previous page (page up)
   { {NULL}, NULL }        // terminator entry
};

typedef struct keymap_test {
   KEYMAP     base;
   TCE_ACTION *keys;
} KM_TEST;

ARV keycall_func(DPARMS *parms, KEYMAP *km, const char *keystroke)
{
   KM_TEST* kmt = (KM_TEST*)km;
   TCE_ACTION *ptr = kmt->keys;
   while (ptr->entry.name)
   {
      if (strcmp(ptr->entry.value, keystroke)==0)
         return (*ptr->action)(parms);
      ++ptr;
   }
   return ARV_CONTINUE;
}

int handle_keystroke(const char *kstk, TCE_ACTION *ta_keys, DPARMS *parms)
{
   TCE_ACTION *ptr = ta_keys;
   while (ptr->entry.name)
   {
      if (strcmp(ptr->entry.value, kstk)==0)
      {
         (*ptr->action)(parms);
         return 1;
      }
      ++ptr;
   }
   return 0;
}

void prepare_DPARMS(DPARMS *parms, LINDEX *index)
{
   memset(parms, 0, sizeof(DPARMS));
   pager_init_dparms(parms, index, index->count, lindex_print);
   pager_set_margins(parms, 4, 4, 4, 4);
}

void prepare_KM_TEST(KM_TEST *km)
{
   fill_termcap_array((TCENTRY*)keys, sizeof(TCE_ACTION));
   km->base.kc_func = keycall_func;
   km->keys = keys;
}

int run_with_keymap(const char *file)
{
   LINDEX *lindex = NULL;

   if (file)
   {
      FILE *fstream = fopen(file, "r");
      if (fstream)
      {
         printf("Opening file %s.\n", file);
         lindex = index_lines(fstream);
         fclose(fstream);
      }
      else
      {
         printf("Failed to open file '%s'.\n", file);
         return 1;
      }
   }
   else
   {
      printf("Using stdin for content.\n");
      lindex = index_lines(stdin);

      // After exhausing stdin, restart it for user input:
      __attribute__((unused))
      FILE* kbtty = freopen("/dev/tty", "r", stdin);
   }

   if (lindex)
   {
      DPARMS parms;
      prepare_DPARMS(&parms, lindex);
      KM_TEST km_test;
      prepare_KM_TEST(&km_test);

      // // Fill screen with 'E's to debug pager coverate
      // ti_write_str("\x1b#8");

      pager_begin(&parms, (KEYMAP*)&km_test, get_keystroke);

      destroy_lindex(lindex);
   }

   return 0;
}

int main(int argc, const char **argv)
{
   int rval = 0;

   // printf("Pausing to permit deubgger attachment.");
   // get_keystroke(NULL,0);

   ti_start_term();
   ti_hide_cursor();
   rval = run_with_keymap(argv[1]);
   ti_show_cursor();
   ti_cleanup_term();

   return rval;
}

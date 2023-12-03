#include <stdio.h>    // for printf, etc
#include <string.h>   // for strcmp, etc
#include <curses.h>
#include <term.h>
#include <unistd.h>   // getpid()

// #include "pager/get_keystroke.h"
// #include "screen.h"
#include "lines.h"
#include "pager/terminal.h"
#include "pager/pager.h"


void open_pager(FILE *strm)
{
   if (strm)
   {
      LINDEX *lindex = index_lines(strm);
      if (lindex)
      {
         printf("There are %d lines in the file.\n", lindex->count);
         char buff[80];
         get_keystroke(buff, sizeof(buff));

         update_keymap_values(sel_keymap, sizeof(PKMAP));

         reset_screen();

         DPARMS dparms;
         initialize_dparms(&dparms, (void*)lindex, lindex->count, lindex_line_printer);
         set_screen_margins(&dparms, 2, -1, -1, -1);

         start_pager(&dparms);

         destroy_lindex(lindex);
      }
      else
         printf("Failed to load the file.\n");
   }
}

// struct tce_keymap {
//    TCENTRY tce;
//    char desc[40];
// };

// void test_fill_keymap_array(void)
// {
//    struct tce_keymap entries[] = {
//       { { "kcud1" }, "down-arrow key" },            // down key
//       { { "kcuu1" }, "up-arrow key" },              // up key
//       { { "khome" }, "home key" },                  // home key
//       { { "kend" }, "end key" },                    // end key
//       { { "knp" }, "next-page key (pgdn)" },        // next-page key (pgdn)
//       { { "kpp" }, "previous-page key (pgup)" },    // previous-page key (pgup)
//       { {NULL} }
//    };

//    fill_termcap_array((TCENTRY*)&entries, sizeof(struct tce_keymap));

//    char tbuff[24];
//    struct tce_keymap *ptr = entries;
//    while (ptr->tce.name)
//    {
//       printf("name: '%s'  value: '%s', desc: '%s'\n",
//              ptr->tce.name,
//              transform_keystroke(tbuff, sizeof(tbuff), ptr->tce.value, NULL),
//              ptr->desc
//          );

//       ++ptr;
//    }
// }


int main(int argc, const char **argv)
{
   // get_terminfo_values();
   // char buff[24];
   // int bufflen = sizeof(buff);
   reset_screen();
   const char *filename = NULL;


   if (argc > 1)
   {
      const char **ptr = argv + 1;
      const char **pend = ptr + argc - 1;

      while (ptr < pend)
      {
         if (**ptr == '-')
         {
            // Leave stub for future option handling:
            switch((*ptr)[1])
            {
               default:
                  printf("Unknown option '%s'\n", *ptr);
                  break;
            }
         }
         else
            filename = *ptr;

         ++ptr;
      }
   }


   if (filename)
   {
      FILE *strm = fopen(filename, "r");
      if (strm)
      {
         open_pager(strm);
         fclose(strm);
      }
   }
   else
      open_pager(stdin);

   return 0;
}


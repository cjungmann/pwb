#include <stdio.h>    // for printf, etc
#include <string.h>   // for strcmp, etc
#include <curses.h>
#include <term.h>
#include <unistd.h>   // getpid()

#include "get_keystroke.h"
#include "terminal.h"
#include "screen.h"
#include "lines.h"
#include "pager.h"

void demo_attributes(void)
{
   printf("About to test ");
   set_bold_mode();
   printf("bold");
   set_normal_mode();
   printf("\n");

   printf("About to test ");
   set_italic_mode();
   printf("italic");
   set_normal_mode();
   printf("\n");

   printf("About to test ");
   set_reverse_mode();
   printf("reverse");
   set_normal_mode();
   printf("\n");

   printf("About to test ");
   start_standout_mode();
   printf("standout");
   stop_standout_mode();
   printf("\n");
}

void report_screensize(void)
{
   int rows, cols;
   get_screen_size(&rows, &cols);
   printf("The screen size is %d rows and %d columns.\n", rows, cols);
}

void test_llist(void)
{
   const char* strings[]={
      "Yo mama",
      "Yo papa",
      "Yo brother",
      "Yo sister",
      "Yo cousin",
      "Yo grandpa",
      "Yo grandma",
      NULL
   };

   LLIST *llist_head = NULL;
   LLIST *llist_tail = NULL;

   const char **ptr = strings;
   APPEND_LLIST(&llist_head, NULL, *ptr, strlen(*ptr));
   llist_tail = llist_head;
   while (*++ptr != NULL)
      APPEND_LLIST(&llist_tail, llist_tail, *ptr, strlen(*ptr));

   int len = get_llist_content_length(llist_head);
   printf("The length of the string is %d.\n", len);
   char *buff = (char*)alloca(len+1);
   concatenate_llist(buff, len+1, llist_head);
   printf("The concatenated list is:\n'%s'\n", buff);
}

void test_read_file(void)
{
   FILE *strm = fopen("main.c", "r");
   if (strm)
   {
      LINDEX *lindex = index_lines(strm);
      if (lindex)
      {
         const char **ptr = lindex->larray;
         const char **end = ptr + lindex->count;

         int count = 0;
         while (ptr < end)
         {
            printf("%d: \x1b[32;1m%s\x1b[m\n", ++count, *ptr);
            ++ptr;
         }

         destroy_lindex(lindex);
      }
   }
}

void open_pager(FILE *strm)
{
   if (strm)
   {
      LINDEX *lindex = index_lines(strm);
      if (lindex)
      {
         update_keymap_values(sel_keymap, sizeof(PKMAP));

         reset_screen();
         DPARMS dparms = { 2, 2, 2, 2,
            0, 0,
            get_lindex_row_count(lindex),
            (void*)lindex,
            lindex_line_printer,
            sel_actionmap,
            sel_keymap
         };

         start_pager(&dparms);

         destroy_lindex(lindex);
      }
   }
}

void demo_pager(void)
{
   FILE *strm = fopen("main.c", "r");
   if (strm)
   {
      LINDEX *lindex = index_lines(strm);
      if (lindex)
      {
         update_keymap_values(sel_keymap, sizeof(PKMAP));

         reset_screen();
         DPARMS dparms = { 2, 2, 2, 2,
            0, 0,
            get_lindex_row_count(lindex),
            (void*)lindex,
            lindex_line_printer,
            sel_actionmap,
            sel_keymap
         };

         start_pager(&dparms);

         destroy_lindex(lindex);
      }
   }
}

struct tce_keymap {
   TCENTRY tce;
   char desc[40];
};

void test_fill_keymap_array(void)
{
   struct tce_keymap entries[] = {
      { { "kcud1" }, "down-arrow key" },            // down key
      { { "kcuu1" }, "up-arrow key" },              // up key
      { { "khome" }, "home key" },                  // home key
      { { "kend" }, "end key" },                    // end key
      { { "knp" }, "next-page key (pgdn)" },        // next-page key (pgdn)
      { { "kpp" }, "previous-page key (pgup)" },    // previous-page key (pgup)
      { {NULL} }
   };

   fill_termcap_array((TCENTRY*)&entries, sizeof(struct tce_keymap));

   char tbuff[24];
   struct tce_keymap *ptr = entries;
   while (ptr->tce.name)
   {
      printf("name: '%s'  value: '%s', desc: '%s'\n",
             ptr->tce.name,
             transform_keystroke(tbuff, sizeof(tbuff), ptr->tce.value, NULL),
             ptr->desc
         );

      ++ptr;
   }
}

void test_fill_termcap_array(void)
{
   TCENTRY entries[] = {
      { "kcud1" },     // down key
      { "kcuu1" },     // up key
      { "khome" },     // home key
      { "kend" },      // end key
      { "knp" },       // next-page key (pgdn)
      { "kpp" },       // previous-page key (pgup)
      { NULL }
   };

   fill_termcap_array((TCENTRY*)&entries, sizeof(TCENTRY));

   char tbuff[24];

   TCENTRY *ptr = entries;
   while (ptr && ptr->name)
   {
      printf("name: '%s'  value: '%s'\n",
             ptr->name,
             transform_keystroke(tbuff, sizeof(tbuff), ptr->value, NULL));
      ++ptr;
   }
}

int main(int argc, const char **argv)
{
   get_terminfo_values();
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
            switch((*ptr)[1])
            {
               case 'c':
                  demo_attributes();
                  break;
               case 'e':
                  test_fill_termcap_array();
                  break;
               case 'f':
                  test_read_file();
                  break;
               case 'k':
                  test_fill_keymap_array();
                  break;
               case 'l':
                  test_llist();
                  break;
               case 'p':
                  demo_pager();
                  break;
               case 's':
                  report_screensize();
                  break;
               case 't':
                  print_full_list();
                  break;
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


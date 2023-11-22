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
   char buff[24];
   int bufflen = sizeof(buff);
   reset_screen();

   while (true)
   {
      if (get_transformed_keystroke(buff, bufflen, NULL))
      {
         if (strcmp(buff, "q")==0)
            break;
         else if (strcmp(buff, "e")==0)
            test_fill_termcap_array();
         else if (strcmp(buff, "k")==0)
            test_fill_keymap_array();
         else if (strcmp(buff, "t")==0)
            print_full_list();
         else if (strcmp(buff, "c")==0)
            demo_attributes();
         else if (strcmp(buff, "i")==0)
            printf("Process ID: %d\n", getpid());
         else if (strcmp(buff, "l")==0)
            test_llist();
         else if (strcmp(buff, "f")==0)
            test_read_file();
         else if (strcmp(buff, "p")==0)
            demo_pager();
         else if (strcmp(buff, "s")==0)
         {
            int rows, cols;
            get_screen_size(&rows, &cols);
            printf("The screen size is %d rows and %d columns.\n", rows, cols);
         }
         else
         {
            printf("You pressed ");
            set_bold_mode();
            printf("'%s'", buff);
            set_normal_mode();
            printf("\n");
         }
      }
   }

   return 0;
}


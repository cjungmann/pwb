#include <stdio.h>    // for printf, etc
#include <string.h>   // for strcmp, etc
#include <curses.h>
#include <term.h>

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
         DPARMS dparms = { 2, 2, 2, 2,
            0, 0,
            get_lindex_row_count(lindex),
            (void*)lindex,
            lindex_line_printer
         };

         print_page(&dparms);


         destroy_lindex(lindex);
      }
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
         else if (strcmp(buff, "t")==0)
            print_full_list();
         else if (strcmp(buff, "c")==0)
            demo_attributes();
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


#include <stdio.h>
#include "freader.h"


int main(int argc, const char **argv)
{
   int rval = 1;

   // Remember, arg[0] is the command, we need a value
   // in arg[1]:
   if (argc <= 1)
   {
      printf("You gotta give me a file to parse, bruh.\n");
      goto final_exit;
   }

   const char *filename = argv[1];
   FILE *file = fopen(filename, "r");
   if (!file)
   {
      printf("Dude! '%s' cannot be opened!\n", filename);
      goto final_exit;
   }


   LINDEX *index = index_lines(file);
   if (!index)
   {
      printf("Something bad happened.  We can't go on.\n");
      goto close_before_exit;
   }

   int limit = get_lindex_row_count(index);
   for (int i=0; i<limit; ++i)
   {
      const char *line = get_lindex_line(index, i);
      printf("%s\n", line);
   }
   rval = 0;

   destroy_lindex(index);

  close_before_exit:
   fclose(file);

  final_exit:
   return rval;
}

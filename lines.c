#include "lines.h"

#include <stdio.h>     // for fgets
#include <stdlib.h>   // free()
#include <string.h>   // strlen()
#include <assert.h>

/**
 * @brief Frees list if heap-allocated
 * @param "llist"  pointer to heap-allocated llist
 */
void destroy_llist(LLIST *llist)
{
   LLIST *ptr = llist;
   LLIST *next;
   while(ptr)
   {
      next = ptr->next;

      free(ptr->text);
      free(ptr);

      ptr = next;
   }
}

/**
 * @brief Calculate the length of the list prior to call to @ref concatenate_llist.
 * @param "llist"  pointer to llist
 */
int get_llist_length(const LLIST *llist)
{
   int length = 0;

   const LLIST *ptr = llist;
   while (ptr)
   {
      length += strlen(ptr->text);
      ptr = ptr->next;
   }

   return length;
}

/**
 * @brief Copies contents of @p llist to the @p buff buffer
 * @param "buff"    buffer to which the contents of @p llist are to be copied
 * @param "bufflen" length of @p buff.
 * @param "llist"   pointer to llist from which text will be copied.
 *
 * Submit a buffer and its length to limit the copy length.
 * Use @ref get_llist_length to determine the length of the list.
 * Remember to add an extra byte to the buffer for the terminating '\0'
 * character.
 *
 * The primary purpose of this function is to facilitate collecting
 * sections of a very long text line without committing huge blocks
 * of memory.  Use the macro @ref APPEND_LLIST to make a stack-based
 * list of parts, use @ref get_llist_length to measure the size, allocate
 * sufficient memory, then use this function to copy the parts to the
 * memory.
 */
int concatenate_llist(char *buff, int bufflen, const LLIST *llist)
{
   const LLIST *ptr = llist;

   char *target = buff;
   // Reserve last byte of buffer for '\0'
   char *buff_end = buff + bufflen - 1;

   while(ptr)
   {
      int curlen = strlen(ptr->text);
      char *temp_end = target + curlen;
      if (temp_end < buff_end)
      {
         memcpy(target, ptr->text, curlen);
         target += curlen;
      }
      else
      {
         // If not enough room, copy what fits, update
         // pointers appropiately, and bust out of the loop
         curlen = buff_end - target;
         memcpy(target, ptr->text, curlen);
         target += curlen;
         break;
      }

      ptr = ptr->next;
   }

   // This should be right, but I'm asserting for safety
   assert(target < buff + bufflen);
   *target++ = '\0';

   return target - buff;
}

bool is_end_char(char v) { return v=='\n'; }

char *read_line_from_stream(FILE *stream)
{
   char *line_contents = NULL;

   LLIST *llhead = NULL, *lltail = NULL;
   char linebuff[40];
   int buff_len = sizeof(linebuff);
   bool found_endline = false;

   while (!found_endline && fgets(linebuff, buff_len, stream))
   {
      int slen = strlen(linebuff);
      if (slen == 0)
         goto save_head;
      else if (linebuff[slen-1] == '\n')
      {
         --slen;
         found_endline = true;
      }

      if (lltail != NULL)
      {
         // Macro must be enclosed in curly-braces
         APPEND_LLIST(&lltail, lltail, linebuff, slen);
      }
      else
      {
         APPEND_LLIST(&llhead, NULL, linebuff, slen);
         lltail = llhead;
      }
   }

  save_head:
   if (llhead)
   {
      int linelen = get_llist_length(llhead);
      ++linelen;
      line_contents = (char*)malloc(linelen);
      if (line_contents)
         concatenate_llist(line_contents, linelen, llhead);
   }

   return line_contents;
}

int count_llist_lines(LLIST *llist)
{
   int count = 0;
   while (llist)
   {
      ++count;
      llist = llist->next;
   }

   return count;
}

/**
 * @brief Make a list of strings from lines of text from `stream`.
 */
LLIST* fill_llist(LLIST *llist, FILE *stream)
{
   LLIST *llhead = llist, *lltail = llist, *llnew;
   char *line_text = (char*)1;
   while(line_text)
   {
      line_text = read_line_from_stream(stream);
      if (line_text)
      {
         llnew = (LLIST*)malloc(sizeof(LLIST));
         memset(llnew, 0, sizeof(LLIST));
         llnew->text = line_text;
         if (lltail)
         {
            lltail->next = llnew;
            lltail = llnew;
         }
         else
            llhead = lltail = llnew;
      }
      else
         break;
   }

   return llhead;
}


void destroy_lindex(LINDEX *index)
{
   if (index)
   {
      if (index->larray)
      {
         free(index->larray);
         index->larray = NULL;
      }

      if (index->llist)
      {
         destroy_llist(index->llist);
         index->llist = NULL;
      }
   }
}

bool index_lines(LINDEX **index, FILE *stream)
{
   LLIST *llist = fill_llist(NULL, stream);
   if (llist)
   {
      int count = count_llist_lines(llist);
      if (count > 0)
      {
         char **line_vector = (char**)malloc(count * sizeof(char*));
         LLIST *lptr = llist;
         char **cptr = line_vector;
         while (lptr)
         {
            *cptr = lptr->text;

            lptr = lptr->next;
            ++cptr;
         }

         LINDEX *newindex = (LINDEX*)malloc(sizeof(LINDEX));
         memset(newindex, 0, sizeof(LINDEX));
         newindex->count = count;
         newindex->llist = llist;
         newindex->larray = (const char **)line_vector;

         *index = newindex;
         return true;
      }
   }
   return false;
}


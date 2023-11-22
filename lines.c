/**
 * @file lines.c
 * @brief Set of functions that enable indexed access to the lines of a file.
 */

#include <stdio.h>     // for fgets
#include <stdlib.h>   // free()
#include <string.h>   // strlen()
#include <unistd.h>   // STDOUT_FILENE
#include <assert.h>
#include <errno.h>

#include <curses.h>
#include <term.h>

#include "lines.h"
#include "screen.h"


/**
 * @brief Uses `free` to return allocated memory to the heap.
 * @param "llist"  pointer to llist to be destroyed
 *
 * This function will cause an error if any part of the LLIST
 * is stack memory.  The programmer is respopnsible for ensuring
 * that all allocated parts of the LLIST (the LLIST pointer and
 * its ::text member) are from the heap.
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
 * @return cumulative number of characters in all the list elements.
 */
int get_llist_content_length(const LLIST *llist)
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
 * Use @ref get_llist_content_length to determine the length of the list.
 * Remember to add an extra byte to the buffer for the terminating '\0'
 * character.
 *
 * The primary purpose of this function is to facilitate collecting
 * sections of a very long text line without committing huge blocks
 * of memory.  Use the macro @ref APPEND_LLIST to make a stack-based
 * list of parts, use @ref get_llist_content_length to measure the size, allocate
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

/**
 * @brief Read one line from a file
 * @param "stream"  Open stream handle from which to read
 * @return `malloc`ed memory with the contents of the line.
 *
 * Reads lines of arbitrary length.  Line contents first go into
 * a stack-based LLIST to accommodate unexpected line lengths.
 * Upon finding the newline or EOF, the size of the line is
 * determined, the contents of the line are copied to heap
 * memory and returned.
 */
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
      int linelen = get_llist_content_length(llhead);
      ++linelen;
      line_contents = (char*)malloc(linelen);
      if (line_contents)
         concatenate_llist(line_contents, linelen, llhead);
   }

   return line_contents;
}


/**
 * @brief Returns number of elements in a LLIST list.
 * @param "llist"   Item whose elements are to be enumerated
 * @return number of elements in the LLIST
 *
 * This function is used to help prepare an array of char*
 * for the final index product
 */
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
 * @param "llist"   LLIST object to which new LLIST elements will be appended.
 *                  This is expected to be NULL, but allows for adding to a
 *                  list.
 * @param "stream"  Open stream from which lines will be read
 *
 * This function creates a heap-allocated linked list of strings, each
 * string being one complete line from the @p stream file.
 *
 * Use @ref destroy_llist to dispose of the result.
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


/**
 * @brief Returns memory from LINDEX to the heap
 * @param "index"   Product of the @ref index_lines function
 *
 * Frees all memory managed by the @p index, then the @p index
 * itself.
 */
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

      free(index);
   }
}

/**
 * @brief Return data package includeing linked list and array of text lines.
 * @param "stream"   open stream to text file to be read
 * @return pointer to @ref LINDEX package of text lines
 *
 * First reads text lines into a linked list of text (@ref LLIST),
 * the makes an array of char* pointing to each element of the list
 * to allow indexed access to the content.
 *
 * The resultant struct is a heap item, and should be destroyed using
 * the @ref destroy_lindex function.
 */
LINDEX *index_lines(FILE *stream)
{
   LINDEX *newindex = NULL;

   LLIST *llist = fill_llist(NULL, stream);
   if (llist)
   {
      int count = count_llist_lines(llist);
      if (count == 0)
         destroy_llist(llist);
      else
      {
         char **line_vector = (char**)malloc(count * sizeof(char*));
         if (line_vector == NULL)
            destroy_llist(llist);
         else
         {
            LLIST *lptr = llist;
            char **cptr = line_vector;
            while (lptr)
            {
               *cptr = lptr->text;

               lptr = lptr->next;
               ++cptr;
            }

            newindex = (LINDEX*)malloc(sizeof(LINDEX));
            if (newindex == NULL)
            {
               free(line_vector);
               destroy_llist(llist);
            }
            else
            {
               memset(newindex, 0, sizeof(LINDEX));
               newindex->count = count;
               newindex->llist = llist;
               newindex->larray = (const char **)line_vector;
            }
         }
      }
   }

   return newindex;
}

int get_lindex_row_count(const LINDEX *lindex)
{
   if (lindex && lindex->count)
      return lindex->count;
   else
      return 0;
}

const char *get_lindex_line(const LINDEX *lindex, int index)
{
   if (index<0 || index>=lindex->count)
   {
      errno = EINVAL;
      return NULL;
   }

   return lindex->larray[index];
}

int lindex_line_printer(int row_index, int has_focus, int length, void *dsource)
{
   LINDEX *lindex = (LINDEX*)dsource;
   const char *text = get_lindex_line(lindex, row_index);
   char *buff = (char*)alloca(length+1);
   if (text)
      snprintf(buff, length+1, "%-*s", length, text);
   else
   {
      memset(buff, ' ', length);
      buff[length] = '\0';
   }

   if (has_focus)
      set_reverse_mode();
   write(STDOUT_FILENO, buff, length);
   if (has_focus)
      set_normal_mode();
   return 0;
}

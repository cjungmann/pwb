#ifndef LINES_H
#define LINES_H

#include <stdio.h>
#include <alloca.h>
#include <stdbool.h>

typedef struct line_list LLIST;
typedef struct line_index LINDEX;

struct line_list {
   char  *text;
   LLIST *next;
};

struct line_index {
   int count;            ///< number of elements in @ref llist and @ref larray
   LLIST *llist;         ///< linked list of text lines
   const char **larray;  ///< array of pointers to text in @ref llist elements
};

/**
 * @defgroup Public LINDEX functions for accessing a text file by lines
 *
 * Be sure to use @ref destroy_lindex when done with the content
 * in a LINDEX returned by @ref index_lines.
 * @{
 */
LINDEX *index_lines(FILE *stream);
void destroy_lindex(LINDEX *index);
int get_lindex_row_count(const LINDEX *lindex);
const char *get_lindex_line(const LINDEX *lindex, int index);
/** @} */


/**
 * @defgroup Internal LINDEX functions use to construct and destruct
 *           a @ref LINDEX object.
 */
int count_llist_lines(LLIST *llist);
void destroy_llist(LLIST *llist);
int get_llist_content_length(const LLIST *llist);
int concatenate_llist(char *buff, int bufflen, const LLIST *llist);

LLIST* fill_llist(LLIST *llist, FILE *stream);

/**
 * @brief Macro used by @ref fill_llist for space to process long text lines
 */
#define APPEND_LLIST(llist_out, llist_in, value, vlen)  {  \
   LLIST *newllist = (LLIST*)alloca(sizeof(LLIST));        \
   memset(newllist, 0, sizeof(LLIST));                     \
                                                           \
   newllist->text = (char*)alloca(vlen+1);                 \
   memcpy(newllist->text, (value), vlen);                  \
   newllist->text[vlen] = '\0';                            \
                                                           \
   if ((llist_in) != NULL)                                 \
      ((LLIST*)(llist_in))->next = newllist;               \
   *(llist_out) = newllist;                                \
}

/** @} */


#endif

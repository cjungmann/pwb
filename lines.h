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
   int count;
   LLIST *llist;
   const char **larray;
};

void destroy_llist(LLIST *llist);
int get_llist_content_length(const LLIST *llist);
int concatenate_llist(char *buff, int bufflen, const LLIST *llist);
int count_llist_lines(LLIST *llist);

LLIST* fill_llist(LLIST *llist, FILE *stream);

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


void destroy_lindex(LINDEX *index);
LINDEX *index_lines(FILE *stream);

int get_lindex_row_count(const LINDEX *lindex);
const char *get_lindex_line(const LINDEX *lindex, int index);

int lindex_line_printer(int row_index, int indicated, int length, void *dsource);


#endif

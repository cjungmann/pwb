#ifndef PWB_VERBS_H
#define PWB_VERBS_H

#include "pwb_handle.h"
#include "pwb_result.h"
#include <argeater.h>
#include <pager.h>

typedef PWB_RESULT (*PWB_RUN)(PWBH *handle, ACLONE *args);

typedef struct pwb_action_verb {
   const char *name;
   const char *desc;
   const char *usage;
   PWB_RUN    action;
} PWB_VERB;

PWB_VERB *find_verb(const char *verb);
PWB_RESULT perform_verb(WORD_LIST *wl);

#endif

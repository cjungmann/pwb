#include "pwb_verbs.h"
#include "pwb_actions.h"
#include "pwb_handle.h"
#include <argeater.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

PWB_RESULT pwb_show_help(PWBH *handle, ACLONE *args);

PWB_VERB verbs [] = {
   { "declare",
     "Make a new PWB handle",
     "pwb declare handle_name data_source data_count print_function",
     pwb_action_declare },

   { "help",
     "Show a help display",
     "pwb help",
     pwb_show_help },

   { "start",
     "Begin a pager session",
     "pwb start handle_name top_row focus_row",
     pwb_action_start },

   { "report",
     "Report handle values",
     "pwb report handle_name",
     pwb_action_report },

   // Use after changing a row's contents to update view,
   // especially after an action at the focus row changes
   // remote rows.
   { "print_row",
     "Print a data row by line number",
     "pwb print_row handle_name data_row",
     pwb_action_dummy },

   // Move the focus row by relative amount (+/- N) or to
   // to an absolute data-row index
   { "move",
     "Change focus row",
     "pwb move handle_name +1|-1|1",
     pwb_action_dummy },

   { "scroll",
     "Change screen contents",
     "pwb scroll handle_name +1|-1|1",
     pwb_action_dummy }
};

int verb_count = sizeof(verbs) / sizeof(PWB_VERB);

PWB_RESULT pwb_show_help(PWBH *handle, ACLONE *args)
{
   printf("This is the help page.\n");
   return PWB_SUCCESS;
}


PWB_VERB *find_verb(const char *verb)
{
   PWB_VERB *vptr = verbs;
   PWB_VERB *vend = vptr + verb_count;

   while (vptr < vend)
   {
      if (strcmp(verb, vptr->name)==0)
         return vptr;
      ++vptr;
   }

   return (PWB_VERB*)NULL;
}

static const PWB_VERB *declare_verb = &verbs[0];
static const PWB_VERB *help_verb = &verbs[1];
// assert(strcmp(declare_verb->name, "declare")==0);
// assert(strcmp(help_verb->name, "help")==0);


PWB_RESULT perform_verb(WORD_LIST *wl)
{
   PWB_RESULT result = PWB_UNKNOWN_VERB;

   PWB_VERB *verb = NULL;
   PWBH *handle = NULL;
   ACLONE *argclones = NULL;

   while (wl)
   {
      if (!verb)
      {
         verb = find_verb(wl->word->word);
         if (!verb)
         {
            result = PWB_UNKNOWN_VERB;
            goto take_leave;
         }

         // Skip handle search for verbs  "help" and "declare"
         if (verb == help_verb || verb == declare_verb)
         {
            if (verb == declare_verb)
            {
               // Having the verb, increment WORD_LIST to get
               // the first argument past the verb:
               wl = wl->next;

               argclones = CLONE_WORD_LIST(wl);
            }
            // Proceed to run verb's action without a handle:
            goto run_action;
         }
      }
      else if (!handle)
      {
         handle = pwb_get_handle_from_name(wl->word->word);
         if (!handle)
         {
            result = PWB_UNKNOWN_HANDLE;
            goto take_leave;
         }
      }
      else
      {
         argclones = CLONE_WORD_LIST(wl);
         goto take_leave;
      }

      wl = wl->next;
   }

  run_action:
   result = (*verb->action)(handle, argclones);

  take_leave:
   return result;
}

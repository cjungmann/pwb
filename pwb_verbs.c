#include "pwb_verbs.h"
#include "pwb_actions.h"
#include "pwb_handle.h"
#include "pwb_errors.h"
#include <argeater.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

PWB_RESULT pwb_show_help(PWBH *handle, ACLONE *args);

PWB_VERB verbs [] = {
   { "help",
     "Show a help display",
     "pwb help",
     pwb_show_help },

   { "init",
     "Save terminal state and prepare for pager",
     "pwb init",
     pwb_action_init },

   { "restore",
     "Restore initial terminal state",
     "pwb restore",
     pwb_action_restore },

   { "declare",
     "Make a new PWB handle",
     "pwb declare handle_name data_source data_count print_function",
     pwb_action_declare },

   { "declare_keymap",
     "Make a keymap from an array",
     "pwb create_keymap handle_name array_name",
     pwb_action_declare_keymap },

   /**
    * This marks the border between actions that take no handle,
    * either because it doesn't need one (init, restore, help), or
    * because it creates one (declare, declare_keymap).
    */
#define VERB_NEEDS_HANDLE "start"

   { "start",
     "Begin a pager session",
     "pwb start handle_name top_row focus_row",
     pwb_action_start },

   { "set_margins",
     "Set margins of active area",
     "pwb margins top [right] [bottom] [left]",
     pwb_action_set_margins },

   { "report",
     "Report handle values",
     "pwb report handle_name",
     pwb_action_report },

   { "trigger",
     "Cause an action to be executed",
     "pwb trigger handle_name index [keystroke]",
     pwb_action_trigger },

   // Use after changing a row's contents to update view,
   // especially after an action at the focus row changes
   // remote rows.
   { "print_row",
     "Print a data row by line number",
     "pwb print_row handle_name data_row",
     pwb_action_plot_line },

   { "print_all",
     "Print full screen of data",
     "pwb print_all handle_name",
     pwb_action_plot_screen },

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


// Setup verb test on first call only
bool test_verb_waives_handle_setup(const PWB_VERB *vptr);

// Global variables for testing verbs:
bool (*verb_waives_handle)(const PWB_VERB *vptr) = test_verb_waives_handle_setup;
PWB_VERB *verb_needing_handle = NULL;

bool test_verb_waives_handle(const PWB_VERB *vptr)
{
   return vptr && (vptr < verb_needing_handle);
}

bool test_verb_waives_handle_setup(const PWB_VERB *vptr)
{
   assert(verb_needing_handle==NULL);
   verb_needing_handle = find_verb(VERB_NEEDS_HANDLE);
   assert(verb_needing_handle);

   verb_waives_handle = test_verb_waives_handle;
   return (*verb_waives_handle)(vptr);
}

PWB_RESULT pwb_show_help(PWBH *handle, ACLONE *args)
{
   printf("This is the help page.\n");
   return PWB_SUCCESS;
}

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
            (*error_sink)("Unknown verb '%s'.", wl->word->word);
            goto take_leave;
         }

         if ((*verb_waives_handle)(verb))
         {
            // Ignore missing handle: collect remaining arguments and run verb
            argclones = CLONE_WORD_LIST(wl->next);
            break;
         }
      }
      else if (!handle)
      {
         handle = pwb_get_handle_from_name(wl->word->word);
         if (!handle)
         {
            result = PWB_UNKNOWN_HANDLE;
            (*error_sink)("Unrecognized PWB handle name '%s'.", wl->word->word);
            goto take_leave;
         }
      }
      else
      {
         argclones = CLONE_WORD_LIST(wl);
         break;
         // goto run_action;
      }

      wl = wl->next;
   }

  // run_action:
   result = (*verb->action)(handle, argclones);

  take_leave:
   return result;
}

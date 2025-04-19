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

   { "version",
     "Show the current version number",
     "pwb version",
     pwb_action_version },

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

   { "measure_string",
     "Report printed length of string",
     "pwb measure_string return_name string_name",
     pwb_action_measure_string },

   { "limit_print",
     "Print string to limit of screen characters",
     "pwb limit_print count string_name",
     pwb_action_limit_print },

   { "get_keystroke",
     "Returns user's next keystroke",
     "pwb get_keystroke",
     pwb_action_get_keystroke },

   { "get_dimensions",
     "get screen, margin, and window dimensions",
     "pwb git_dimensions handle_name [associative_array_name]",
     pwb_action_get_dims },

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
     "pwb set_margins handle_name top [right] [bottom] [left]",
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

   { "get_data_source",
     "Get the name of the data source",
     "pwb get_data_source handle_name [-v varname]",
     pwb_action_get_data_source },

   { "get_data_count",
     "Get count of elements of data source",
     "pwb get_data_count handle_name [-v varname]",
     pwb_action_get_data_count },

   { "set_data_count",
     "Set new value for number of rows in data source",
     "pwb update_data_count handle_name new_count",
     pwb_action_set_data_count },

   { "get_top_row",
     "Get row index of top line in content area",
     "pwb get_top_row handle_name [-v varname]",
     pwb_action_get_top_row },

   { "get_focus_row",
     "Get row index of focus line in content area",
     "pwb get_focus_row handle_name [-v varname]",
     pwb_action_get_focus_row },

   { "set_focus_row",
     "Set row index of focus and/or top line in content area",
     "pwb get_focus_row handle_name [-f focus_row] [-t top_row]",
     pwb_action_set_focus_row },

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

/**
 * @brief Primary function called from pwb_builtin
 *
 * All PWB action comes through this function, where the calling
 * arguments request specific actions.
 */
PWB_RESULT perform_verb(WORD_LIST *wl)
{
   PWB_RESULT result = PWB_UNKNOWN_VERB;

   PWB_VERB *verb = NULL;
   PWBH *handle = NULL;
   ACLONE *argclones = NULL;
   int handle_waived = 0;

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
            // Set flag in case no remaining arguments
            handle_waived = 1;
            // Ignore missing handle: collect remaining arguments and run verb
            argclones = CLONE_WORD_LIST(wl->next);
            break;
         }
      }
      else if (!handle)
      {
         // Make sure the handle name is at least 1 character long
         // (or pwb_get_handle_from_name will fail):
         const char *handle_name = wl->word->word;
         if (handle_name && *handle_name)
         {
            handle = pwb_get_handle_from_name(handle_name);
            if (!handle)
            {
               result = PWB_UNKNOWN_HANDLE;
               (*error_sink)("Unrecognized PWB handle name '%s'.", handle_name);
               goto take_leave;
            }
         }
         else
         {
            result = PWB_INVALID_ARGUMENT;
            (*error_sink)("Empty string passed for handle name.");
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

   if (!handle && !handle_waived)
   {
      result = PWB_MISSING_HANDLE_NAME;
      (*error_sink)("Missing handle name.");
   }
   else
      // run_action:
      result = (*verb->action)(handle, argclones);

  take_leave:
   return result;
}

#include "pwb_keymap.h"
#include <pager.h>
#include <argeater.h>
#include <alloca.h>
#include <errno.h>

ARV run_quit(PWBH *handle, WORD_LIST *word_list)
{
   return ARV_EXIT;
}

ARV run_down_arrow(PWBH *handle, WORD_LIST *word_list)
{
   return pager_focus_down_one(&handle->dparms);
}

ARV run_up_arrow(PWBH *handle, WORD_LIST *word_list)
{
   return pager_focus_up_one(&handle->dparms);
}

ARV run_down_page(PWBH *handle, WORD_LIST *word_list)
{
   return pager_focus_down_page(&handle->dparms);
}

ARV run_up_page(PWBH *handle, WORD_LIST *word_list)
{
   return pager_focus_up_page(&handle->dparms);
}

ARV run_end(PWBH *handle, WORD_LIST *word_list)
{
   return pager_focus_end(&handle->dparms);
}

ARV run_home(PWBH *handle, WORD_LIST *word_list)
{
   return pager_focus_home(&handle->dparms);
}

ARV run_execute(PWBH *handle, WORD_LIST *word_list)
{
   pwb_execute_command(word_list);
   return ARV_CONTINUE;
}


PWB_KEYACT action_table[] = {
   run_quit,
   run_down_arrow,
   run_up_arrow,
   run_down_page,
   run_up_page,
   run_end,
   run_home,
   run_execute
};

int action_table_count = sizeof(action_table) / sizeof(PWB_RUN);

PWB_KEYACT get_keyact(unsigned int index)
{
   if (index < action_table_count)
      return action_table[index];
   else
      return NULL;
}

int pwb_get_kdata_size(const char **els, int el_count, const char *label)
{
   int label_len = label ? strlen(label) : 0;
   int total_bytes = sizeof(int) + 1 + label_len;

   const char **ptr = els;
   const char **end = ptr + el_count;

   int entry_count = 0;

   const char *keystroke = NULL;
   const char *index_str = NULL;

   while (ptr < end)
   {
      if (!keystroke)
         keystroke = *ptr;
      else if (!index_str)
      {
         ++entry_count;
         total_bytes += sizeof(int) + 1 + strlen(keystroke);
         keystroke = NULL;
         index_str = NULL;
      }
      ++ptr;
   }

   return total_bytes + sizeof(struct keymap_entry) * entry_count;
}

PWB_RESULT initialize_kclass(KCLASS *kclass, KDATA *kdata)
{
   if (kdata != NULL)
   {
      memset(kclass, 0, sizeof(KCLASS));
      char *free = (char*)kdata;
      int label_len = strlen(free);
      free += label_len + 1;

      kclass->data = kdata;
      kclass->entry_count = (int*)free;
      free += sizeof(int);

      kclass->entries = (KENTRY*)free;

      return PWB_SUCCESS;
   }

   return PWB_OUT_OF_MEMORY;
}


const char *default_keymap[] = {
   "q",       "0",  // quit
   "\033OB",  "1",  // focus down
   "\033OA",  "2",  // focus up
   "\033[6~", "3",  // focus page down
   "\033[5~", "4",  // focus page up
   "\033OF",  "5",  // end
   "\033OH",  "6",  // home
   "\x0d",    "7"   // control-M == ENTER for execute
};

int default_keymap_len = sizeof(default_keymap) / sizeof(const char *);

/**
 * @defgroup Manage singleton allocation of default keymap
 * @{
 */
KDATA *get_new_default_kdata(void);
PWB_GET_DEF_KDATA get_default_kdata = get_new_default_kdata;

KDATA *default_kdata = NULL;

KDATA *get_allocated_default_kdata(void)
{
   return default_kdata;
}

KDATA *get_new_default_kdata(void)
{
   KDATA *kdata = NULL;

   int len = pwb_get_kdata_size(default_keymap, default_keymap_len, KEYMAP_LABEL);
   if (len)
   {
      kdata = get_kdata_from_array(default_keymap,
                                   default_keymap_len,
                                   KEYMAP_LABEL,
                                   malloc,
                                   free);

      if (kdata)
      {
         // Install global default keymap KDATA
         default_kdata = kdata;

         // Update function pointer
         get_default_kdata = get_allocated_default_kdata;
         // Call the newly-installed kdata-getter in case it
         // becomes less than trivial:
         kdata = (*get_default_kdata)();
      }
   }

   return kdata;
}


/** @} */

// void dump_keymap(void)
// {
//    const char **ptr = (const char **)default_keymap;
//    const char **end = ptr + default_keymap_len;

//    const char *keystroke = NULL;
//    while (ptr < end)
//    {
//       if (!keystroke)
//          keystroke = *ptr;
//       else
//       {
//          if (*keystroke == '\x1b')
//          {
//             printf("^E");
//             ++keystroke;
//          }
//          printf("%8s: %s\n", keystroke, *ptr);
//          keystroke = NULL;
//       }

//       ++ptr;
//    }
// }

// dump_keymap();


PWB_RESULT pwb_set_keymap_buffer(char *buffer,
                                 int buff_len,
                                 int entry_count,
                                 const char **els,
                                 int el_count,
                                 const char *label)
{
   PWB_RESULT result = PWB_FAILURE;

   char *free = buffer;
   char *buffend = free + buff_len;

   // Copy label, or place \0 in its place, and update free pointer
   int slen;
   if (label && (slen = strlen(label))>0)
   {
      memcpy(free, label, slen);
      free += slen;
   }
   *free = '\0';
   ++free;

   // Copy entry count and update free pointer
   *(int*)free = entry_count;
   free += sizeof(int);

   // If sufficient memory remains, allocate portion to entry array:
   if (free + (entry_count * sizeof(struct keymap_entry)) < buffend)
   {
      // Set array buffer limits, then set free pointer past the entries:
      struct keymap_entry *cur_entry = (struct keymap_entry*)free;
      struct keymap_entry *end_entry = cur_entry + entry_count;
      free += entry_count * sizeof(KENTRY);

      const char **ptr = els;
      const char **end = ptr + el_count;

      const char *keystroke = NULL;

      // Collect paired keystroke strings and action indicies:
      while (ptr < end)
      {
         if (!keystroke)
            keystroke = *ptr;
         else
         {
            if (cur_entry >= end_entry)
            {
               result = PWB_KEY_ENTRY_OVERFLOW;
               goto error_exit;
            }

            int slen = strlen(keystroke);
            if (free + slen + 1 < buffend)
            {
               cur_entry->action_index = atoi(*ptr);

               // Save pointer to string before moving the heap pointer
               cur_entry->keystroke = free;
               // Copy the keystroke string to allocated memory
               memcpy(free, keystroke, slen);
               free[slen] = '\0';
               free += slen + 1;

               ++cur_entry;
               keystroke = NULL;
            }
            else
            {
               result = PWB_OUT_OF_MEMORY;
               goto error_exit;
            }
         }
         ++ptr;
      }

      if (ptr == end && cur_entry == end_entry)
         result = PWB_SUCCESS;
   }
   else
      result = PWB_OUT_OF_MEMORY;

  error_exit:
   return result;
}

KDATA *get_kdata_from_array(const char **els,
                            int els_count,
                            const char *label,
                            void*(*mem_getter)(size_t),
                            void(*mem_freer)(void*))
{
   errno = 0;
   KDATA *retval = (KDATA*)NULL;

   if (els_count>0 && els_count%2 == 0)
   {
      int entry_count = els_count / 2;
      int buff_len = pwb_get_kdata_size(els, els_count, label);
      char *buffer = (char*)(*mem_getter)(buff_len);

      if (buffer)
      {
         PWB_RESULT result = pwb_set_keymap_buffer(buffer,
                                                   buff_len,
                                                   entry_count,
                                                   els,
                                                   els_count,
                                                   label);

         if (result == PWB_SUCCESS)
            retval = (KDATA*)buffer;
         else if (mem_freer)
            (*mem_freer)(buffer);
      }
      else
         errno = ENOMEM;
   }
   else
      errno = EINVAL;

   return retval;
}

PWB_RESULT get_default_keymap(KCLASS *kclass,
                              const char *label,
                              void*(*mem_getter)(size_t),
                              void(*mem_freer)(void*))
{
   KDATA *data = get_kdata_from_array(default_keymap,
                                      default_keymap_len,
                                      label,
                                      mem_getter,
                                      mem_freer);

   return initialize_kclass(kclass, data);
}

PWB_RESULT pwb_make_kdata_shell_var(const char *name,
                                    const char *array_name,
                                    const char *label)
{
   PWB_RESULT result = PWB_FAILURE;

   SHELL_VAR *sv = find_variable(array_name);
   if (sv)
   {
      if (array_p(sv))
      {
         ARRAY *array = array_cell(sv);
         int els_count = array_num_elements(array);
         if (els_count > 1 && (els_count % 2 == 0))
         {
            const char **els = (const char **)alloca(els_count * sizeof(char*));
            const char **ptr = els;
            const char **end = ptr + els_count;

            ARRAY_ELEMENT *ahead = array->head;
            ARRAY_ELEMENT *ael = ahead->next;

            // Copy Bash array to char* array:
            while (ael != ahead && ptr < end)
            {
               *ptr = ael->value;
               ++ptr;
               ael = ael->next;
            }

            if (ael == ahead && ptr == end)
            {
               KDATA *data = get_kdata_from_array(els, els_count, label, xmalloc, xfree);
               if (data)
               {
                  SHELL_VAR *sv_keymap = bind_variable(name, "", 0);
                  if (sv_keymap)
                  {
                     pwb_dispose_variable_value(sv_keymap);
                     sv_keymap->value = (char*)data;
                     sv_keymap->attributes |= att_special;
                     result = PWB_SUCCESS;
                  }
                  else
                     result = PWB_FAILED_BIND;
               }
               else
                  result = PWB_OUT_OF_MEMORY;
            }
            else
               result = PWB_FAILED_KEYMAP_PARSING;
         }
         else
            result = PWB_FAILED_KEYMAP_PARSING;
      }
      else
         result = PWB_INVALID_VARIABLE;
   }
   else
      result = PWB_UNKNOWN_VARIABLE;

   return result;
}


/**
 * @brief Find and execute a keymap function matching a keystroke.
 */
ARV pwb_run_keystroke(PWBH *pwbh,
                      const char *keystroke,
                      struct keymap_class* keymap_base,
                      struct keymap_class* keymap_aux)
{
   struct keymap_entry *bptr = keymap_base->entries;
   struct keymap_entry *bend = bptr + *keymap_base->entry_count;

   struct keymap_entry *xptr = NULL, *xend = NULL;

   if (keymap_aux && keymap_aux->entry_count != NULL)
   {
      xptr = keymap_aux->entries;
      xend = xptr + *keymap_aux->entry_count;
   }

   // Prepare to ignore exec action (7) if there is no exec function
   int allowed_index = action_table_count;

   // Prepare exec function arguments word_list:
   WORD_LIST *wl = pwbh->exec_wl;
   if (wl)
   {
      pwbh_exec_set_keystroke(pwbh, keystroke);
      pwbh_exec_update_row_number(pwbh);
   }
   else
      --allowed_index;

   // Find the keyaction
   PWB_KEYACT cur_action = NULL;

   // Search base keymap:
   while (bptr < bend)
   {
      if (strcmp(bptr->keystroke, keystroke)==0)
      {
         int index = bptr->action_index;
         if (index >=0 && index < allowed_index)
         {
            cur_action = action_table[index];
            break;
         }
      }
      ++bptr;
   }

   // If no action yet, search auxiliary keymap, if provided:
   if (!cur_action)
   {
      // fails entry if xptr == xend == NULL (no auxiliary keymap):
      while (xptr < xend)
      {
         if (strcmp(xptr->keystroke, keystroke)==0)
         {
            int index = xptr->action_index;
            if (index >=0 && index < allowed_index)
            {
               cur_action = action_table[index];
               break;
            }
         }
         ++xptr;
      }
   }

   if (cur_action)
      return (*cur_action)(pwbh, wl);

   return ARV_CONTINUE;
}

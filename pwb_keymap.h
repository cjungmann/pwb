#ifndef PWB_KEYMAP_H
#define PWB_KEYMAP_H

#include "pwb_builtin.h"
#include "pwb_handle.h"
#include "pwb_result.h"
#include "pwb_actions.h"

#define KEYMAP_LABEL "PWB_Keymap"

typedef char KDATA;

typedef ARV (*PWB_KEYACT)(PWBH *handle, WORD_LIST *word_list);
typedef KDATA* (*PWB_GET_DEF_KDATA)(void);

extern PWB_GET_DEF_KDATA get_default_kdata;

typedef struct keymap_entry {
   const char* keystroke;
   int action_index;
} KENTRY;

typedef struct keymap_class {
   KDATA *data;
   int *entry_count;
   KENTRY *entries;
} KCLASS;

#define keymap_label_test(var) (0 == strcmp((var)->value, KEYMAP_LABEL))
#define keymap_p(var) (((var)->attributes & att_special) && keymap_label_test((var)))
#define keymap_cell(var) fprintf(stderr, "Use initialize_kclass\n")

PWB_KEYACT get_keyact(unsigned int index);
extern int action_table_count;


int pwb_get_kdata_size(const char **els,
                                int el_count,
                                const char *label);

PWB_RESULT initialize_kclass(KCLASS *kclass, KDATA *data);

KDATA* get_kdata_from_array(const char **els,
                            int els_count,
                            const char *label,
                            void*(*mem_getter)(size_t),
                            void(*mem_freer)(void*));

PWB_RESULT get_default_keymap(KCLASS *kclass,
                              const char *label,
                              void*(*mem_getter)(size_t),
                              void(*mem_freer)(void*));

PWB_RESULT pwb_make_kdata_shell_var(const char *name,
                                    const char *array_name,
                                    const char *label);

ARV pwb_run_keystroke(PWBH *pwbh,
                      const char *keystroke,
                      struct keymap_class* keymap_base,
                      struct keymap_class* keymap_aux);



#endif

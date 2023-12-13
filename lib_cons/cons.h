#ifndef CONS_H
#define CONS_H

#include <stdbool.h>

typedef struct termcap_entry TCENTRY;

struct termcap_entry {
   const char *name;
   const char *value;
};

// from get_keystroke.c
char* get_keystroke(char *buff, int bufflen);
char *transform_keystroke(char *buff,
                          int bufflen,
                          const char *keystroke,
                          const char *esc_str);

char *transform_keystring(char *buff,
                          int bufflen,
                          const char *keystring,
                          const char *esc_str);

// From terminal.c
const char *capname_from_code(const char *capcode);
bool get_termcap_string(const char **value, const char *capcode);
void fill_termcap_array(TCENTRY* array, int entry_len);

// From screen.c



#endif


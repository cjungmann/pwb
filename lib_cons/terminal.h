#ifndef TERMINAL_H
#define TERMINAL_H

#include <termios.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct termcap_entry TCENTRY;

struct termcap_entry {
   const char *name;
   const char *value;
};

bool launch_terminal(void);
void fill_termcap_array(TCENTRY* array, int entry_len);
bool get_termcap_string(const char **value, const char *capcode);
void print_full_list(void);
void set_rawread_mode(struct termios* tos);

// void cf_make_raw(struct termios* tos);
// void set_tios_raw_mode(struct termios* original, int fh);
// void set_tios_read_mode(int fh, int min_chars, int timeout);
// void restore_tios_mode(struct termios* original, int fh);



#endif


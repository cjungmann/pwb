#ifndef TERMINAL_H
#define TERMINAL_H

#include <termios.h>
#include <unistd.h>
#include <stdbool.h>

bool get_termcap_string(const char **value, const char *termcap);

void print_full_list(void);

void cf_make_raw(struct termios* tos);
void set_tios_raw_mode(struct termios* original, int fh);
void set_tios_read_mode(int fh, int min_chars, int timeout);
void restore_tios_mode(struct termios* original, int fh);



#endif


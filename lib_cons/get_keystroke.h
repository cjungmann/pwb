#ifndef GET_KEYSTROKE_H
#define GET_KEYSTROKE_H

#include <stdbool.h>

void set_rawread_mode(struct termios* tos);

char *transform_keystroke(char *buff, int bufflen, const char *keystroke, const char *esc_str);
char *transform_keystring(char *buff, int bufflen, const char *keystring, const char *esc_str);

char* get_keystroke(char *buff, int bufflen);
char* get_transformed_keystroke(char *buff, int bufflen, const char *esc_str);


#endif


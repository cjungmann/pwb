#ifndef GET_KEYSTROKE_H
#define GET_KEYSTROKE_H

#include <stdbool.h>

char* get_keystroke(char *buff, int bufflen);
char* get_transformed_keystroke(char *buff, int bufflen, const char *esc_str);




#endif


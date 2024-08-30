#ifndef PWB_UTILITIES_H
#define PWB_UTILITIES_H

#include <stdbool.h>

int get_string_saved_len(const char *str);

bool pack_string_in_block(const char **string,
                          char **buff_pointer,
                          char *buff_limit,
                          const char *value);

#endif

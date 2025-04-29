#ifndef PWB_UTILITIES_H
#define PWB_UTILITIES_H

#include "pwb_builtin.h"
#include <stdbool.h>

int get_string_saved_len(const char *str);

bool pack_string_in_block(const char **string,
                          char **buff_pointer,
                          char *buff_limit,
                          const char *value);

int get_var_parameters(char *buffer,
                       int bufflen,
                       SHELL_VAR *sv,
                       bool include_context,
                       bool include_attributes);

#endif

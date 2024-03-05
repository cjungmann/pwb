#ifndef PWB_ARGEATER_H
#define PWB_ARGEATER_H

#include "pwb_keymap.h"

bool pwb_argeater_int_setter(const char **target, const char *value);
bool pwb_argeater_unsigned_int_setter(const char **target, const char *value);
bool argeater_pwbh_setter(const char **target, const char *value);
bool argeater_array_name_setter(const char **target, const char *value);
bool argeater_array_setter(const char **target, const char *value);
bool pwb_argeater_function_setter(const char **target, const char *value);

bool argeater_kclass_setter(const char **target, const char *value);

#endif

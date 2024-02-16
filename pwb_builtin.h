#ifndef PWB_BUILTIN_H
#define PWB_BUILTIN_H

#if defined(_STDIO_H) || defined(ARGEATER_H) || defined(PAGER_H)
#  error pwb_builtin.h must be first include in a source file.
#  include <pwb_builtin.h_must_be_first_include_in_source_file>
#endif


#include <bash/builtins.h>
// Prevent multiple inclusion of shell.h:
#ifndef EXECUTION_FAILURE
#include <bash/shell.h>
#endif

extern int execute_command PARAMS((COMMAND *));

#endif


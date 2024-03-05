#ifndef PWB_ERRORS_H
#define PWB_ERRORS_H

#define ERROR_NAME "PWB_ERROR"

typedef void (*type_error_sink)(const char *format, ...);

extern type_error_sink error_sink;
void pwb_error_shell_var(const char *format, ...);
void pwb_error_print(const char *format, ...);

void pwb_error_clear(void);

#endif

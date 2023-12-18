#ifndef TERMSTUFF_H
#define TERMSTUFF_H


void ti_write_str(const char *str);
int ti_printf(const char *fmt, ...);

void ti_reset_screen(void);
void ti_set_cursor_position(int row, int col);
void ti_get_cursor_position(int *row, int *col);

void ti_get_screen_size(int *rows, int *cols);

void ti_set_scroll_limit(int top, int count);

void ti_start_standout(void);
void ti_end_standout(void);

void ti_scroll_forward(void);
void ti_scroll_reverse(void);



#endif

#ifndef SCREEN_H
#define SCREEN_H


void get_terminfo_values(void);
void reset_screen(void);

void set_bold_mode(void);
void set_italic_mode(void);
void set_reverse_mode(void);
void set_normal_mode(void);

void start_standout_mode(void);
void stop_standout_mode(void);

void hide_cursor(void);
void show_cursor(void);
void normal_cursor(void);

void set_cursor_position(int row, int col);
void get_cursor_position(int *row, int *col);
void get_screen_size(int *row, int *col);



#endif

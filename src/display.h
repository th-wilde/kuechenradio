#ifndef _DISPLAY_H_
#define _DISPLAY_H_
#define DISPLAY_LINE_BUFFER 128

extern void display_init();
extern void display_quit();
extern void display_write(int line, char text[]);

#endif
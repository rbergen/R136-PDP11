#include <curses.h>

extern WINDOW *mainscr;

int wherex();
int wherey();
void PrintCentered();

#undef getch

#define getch()         wgetch(mainscr)
#define gotoxy(x, y)    wmove(mainscr, (y), (x))
#define putch(c)        waddch(mainscr, (c))
#define cputs(s)        waddstr(mainscr, (s))
#define clrscr()        erase()

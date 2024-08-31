#include <curses.h>

extern WINDOW *mainscr;

int wherex();
int wherey();
void do_print_centered();

#define PrintCentered(text) do_print_centered((text), mainscr)
#define PrintHeader(text)   do_print_centered((text), stdscr)

/* curses.h defines this, but we need it to read from mainscr */
#undef getch

#define getch()         wgetch(mainscr)
#define gotoxy(x, y)    wmove(mainscr, (y), (x))
#define putch(c)        waddch(mainscr, (c))
#define cputs(s)        waddstr(mainscr, (s))
#define clrscr()        werase(mainscr)

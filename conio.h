#include <curses.h>

extern WINDOW *mainscr;

int wherex();
int wherey();
void do_print_centered();

#define PrintCentered(text) do_print_centered(mainscr, (text))
#define PrintHeader(text)   do_print_centered(stdscr, (text))

/* curses.h defines this, but we need it to use our own keypress routine */
#undef getch

#define getch()         { wrefresh(mainscr); getkeypress(); }
#define gotoxy(x, y)    wmove(mainscr, (y), (x))
#define putch(c)        waddch(mainscr, (c))
#define cputs(s)        waddstr(mainscr, (s))
#define clrscr()        werase(mainscr)

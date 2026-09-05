#include <curses.h>

extern WINDOW *mainscr;

int wherex();
int wherey();
void do_print_centered();
void PrintError();

#define PrintCentered(text) do_print_centered(mainscr, (text))
#define PrintHeader(text)   do_print_centered(stdscr, (text))

/* curses.h defines this, but we need it to use our own keypress routine */
#undef getch

/* 2.11BSD's scroll() goes through wdeleteln(), which blanks the bottom line of
   the window but then marks the line at the cursor as changed instead of the
   line it just blanked. A scroll that isn't followed by more output therefore
   leaves the old bottom line on the screen, and curses believes it isn't there.
   Touching the window before refreshing makes it compare, and repaint, that
   line as well. */
#define getch()         do { touchwin(mainscr); wrefresh(mainscr); \
                             getkeypress(); } while (0)
#define gotoxy(x, y)    wmove(mainscr, (y), (x))
#define putch(c)        waddch(mainscr, (c))
#define cputs(s)        waddstr(mainscr, (s))
#define clrscr()        werase(mainscr)

/* This file contains a very rudimentary translation of a number of curses functions to
   the Borland C++ conio.h interface. It's just enough to make the porting of the MS-DOS
   version of this game to the PDP-11 easier. */

#include <curses.h>

WINDOW *mainscr;

int wherex(void)
{
    int x, y;

    getyx(mainscr, y, x);

    return x;
}

int wherey(void)
{
    int x, y;

    getyx(mainscr, y, x);

    return y;
}


void do_print_centered(text, win)
char *text;
WINDOW *win;
{
    int numspaces;
    char* spaces;

    numspaces = (COLS - strlen(text)) / 2;
    spaces = (char *)malloc(numspaces + 1);

    memset(spaces, ' ', numspaces);
    spaces[numspaces] = 0;
    waddstr(win, spaces);
    free(spaces);

    waddstr(win, text);
}

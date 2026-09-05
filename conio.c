/* This file contains a very rudimentary translation of a number of curses functions to
   the Borland C++ conio.h interface. It's just enough to make the porting of the MS-DOS
   version of this game to the PDP-11 easier. */

#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

WINDOW *mainscr;

int wherex()
{
    int x, y;

    getyx(mainscr, y, x);

    return x;
}

int wherey()
{
    int x, y;

    getyx(mainscr, y, x);

    return y;
}

void do_print_centered(win, text)
WINDOW *win;
char *text;
{
    int numspaces;
    int textlength;
    char *spaces;

    if (win == NULL || text == NULL)
        return;

    /* Keep this subtraction signed: a line that is wider than the screen must
       not turn into a huge number of spaces. */
    textlength = (int)strlen(text);
    numspaces = (COLS - textlength) / 2;

    if (numspaces > 0)
    {
        spaces = (char *)malloc(numspaces + 1);
        if (spaces != NULL)
        {
            memset(spaces, ' ', numspaces);
            spaces[numspaces] = 0;
            waddstr(win, spaces);
            free(spaces);
        }
    }

    waddstr(win, text);
}

/* Reports a message to whatever we have available. During initialization that
   may be neither window, so fall back to stderr rather than dereferencing NULL. */
void PrintError(message)
char *message;
{
    if (mainscr != NULL)
        waddstr(mainscr, message);
    else if (stdscr != NULL)
    {
        waddstr(stdscr, message);
        wrefresh(stdscr);
    }
    else
        fprintf(stderr, "%s", message);
}

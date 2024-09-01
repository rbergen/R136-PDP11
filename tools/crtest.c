#include <stdio.h>
#include <curses.h>

int main()
{
    WINDOW *mainscr;

    initscr();
    erase();
    raw();

    addstr("**** Missiecode: R136 ****\n");
    mainscr = subwin(stdscr, 0, 0, 2, 0);
    refresh();

    waddstr(mainscr, "Hello, world!\n");
    waddstr(mainscr, "Let's try to overwrite this text!");
    waddstr(mainscr, "\rHow are we doing...? \n");
    wrefresh(mainscr);
    wgetch(mainscr);

    delwin(mainscr);
    erase();
    refresh();
    endwin();
}
#include <stdio.h>
#include <sgtty.h>
#include <curses.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/select.h>

#define KEY_UP          -1
#define KEY_DOWN        -2
#define KEY_RIGHT       -3
#define KEY_LEFT        -4
#define KEY_HOME        -5
#define KEY_INSERT      -6
#define KEY_DELETE      -7
#define KEY_END         -8
#define KEY_PAGEUP      -9
#define KEY_PAGEDOWN    -10
#define KEY_STAB        -11
#define KEY_ESCAPE      -12
#define KEY_UNHANDLED   -13

#define TIMEOUT 100000

int HandleEscapeKey()
{
    int ch;
    struct timeval tv;
    struct fd_set readfds;

    /* Set up the timeout */
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    tv.tv_sec = 0;
    tv.tv_usec = TIMEOUT;

    /* Check if more input is available within the timeout */
    ch = select(1, &readfds, NULL, NULL, &tv);

    if (!ch) return KEY_ESCAPE;

    switch (getchar())
    {
    case '[':
        ch = getchar();

        /* Arrow keys */
        if (ch >= 'A' && ch <= 'D') 
            return 'A' - ch - 1;

        if (ch == 'H') return KEY_HOME;

        if (ch == 'Z') return KEY_STAB;

        if (ch >= '1' && ch <= '9')
        {
            if (getchar() == '~')
            {
                /* Home, Insert, Delete, End */
                if (ch >= '1' && ch <= '6')
                    return '1' - ch + KEY_HOME;

                return KEY_UNHANDLED;
            }

            /* Numeric escape codes always end with ~ */
            while (getchar() != '~');
        }
        break;

    case 'O': /* We can get this after an Escape, but we don't handle these. 
                 So, we pull the next character that inevitably follows and move on.*/
        getchar();
        break;
    }

    return KEY_UNHANDLED;
}

int getkeypress()
{
    int ch;
    struct sgttyb old_term, new_term;

    /* Set up terminal for non-blocking input */
    ioctl(STDIN_FILENO, TIOCGETP, &old_term);
    new_term = old_term;
    new_term.sg_flags |= RAW;
    new_term.sg_flags &= ~ECHO;
    ioctl(STDIN_FILENO, TIOCSETP, &new_term);

    ch = getchar();

    if (ch == 27)
        ch = HandleEscapeKey();

    /* Reset terminal to normal mode */
    ioctl(STDIN_FILENO, TIOCSETP, &old_term);

    return ch;
}

int main()
{
    int input;
    
    initscr();
    erase();
    refresh();
    scrollok(stdscr, TRUE);

    while(TRUE)
    {
        input = getkeypress();

        if (input == 3)
        {
            printw("You pressed: CTRL+C, exiting...\n");
            refresh();
            break;
        }

        printw("You pressed: %d = %c\n", input, input == 27 || input == 13 || input < 0 ? '_' : input);
        refresh();
    }

    endwin();

    return 0;
}
#include <sgtty.h>
#include <stdio.h>
#include <unistd.h>
#include <varargs.h>
#include <sys/select.h>
#include <sys/time.h>
#include "lib.h"

/* 
 * In 2.11BSD K&R C on the PDP-11, vsscanf() is missing. This version is implemented exactly the
 * way I think it would have been based on the implementation of scanf, fscanf and sscanf in stdio.
 * As is the case in the stdio-supplied sscanf, the actual work is done by a library-internal
 * _doscan() function. Its first argument is pointer to a FILE structure, in which the string to 
 * read from is loaded.
 */
int vsscanf(str, fmt, args)
char *str, *fmt;
va_list args;
{
    FILE _strbuf;

    _strbuf._flag = _IOREAD|_IOSTRG;
    _strbuf._ptr = _strbuf._base = str;
    _strbuf._cnt = 0;
    while (*str++)
        _strbuf._cnt++;
    _strbuf._bufsiz = _strbuf._cnt;
    return _doscan(&_strbuf, fmt, args);
}

#define TIMEOUT 100000

/* Function to figure out what is really going on when getkeypress() reads Escape (ASCII 27). 
   Basically meant for use by getkeypress only. */
int _handle_escape_key()
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
                /* Home, Insert, Delete, End, PgUp, PgDn */
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

/*
 * Slightly more useable version of getchar() that understands Escape and certain 
 * escape key sequences as generated in the context of vt320. It responds to 
 * Ctrl-C by exiting the program with a friendly message.
 */
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
        ch = _handle_escape_key();

    if (ch == 3) /* Ctrl-C */
    {
        puts("Bye!");
        exit(0);
    }

    /* Reset terminal to normal mode */
    ioctl(STDIN_FILENO, TIOCSETP, &old_term);

    return ch;
}

/*
 * Simple version of memmove(), which is missing in 2.11BSD K&R C. Specifically
 * meant to copy memory when source and destination overlap.
 */
void memshift(dest, src, len)
char *dest, *src;
int len;
{
    if (dest == src || len == 0)
        return;

    if (dest < src)
        while (len--)
            *dest++ = *src++;
    else
    {
        dest += len;
        src += len;
        while (len--)
            *--dest = *--src;
    }
}

/*
 * VERY simple text obfuscation function. It's not meant to be secure, just
 * to make it a bit harder to read the strings in the data files that come
 * with the game. It basically inverts each character in the ASCII printable
 * range within the boundaries of that range (32-126, or "space" to ~).
 * Problems are likely if the input string contains characters outside that
 * range, but none of the game strings do.
 */
void fuzzle(str)
char *str;
{
    for (; *str && *str != '\n'; str++)
        *str = 158 - *str;
}

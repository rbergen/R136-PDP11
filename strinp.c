/***************************************************************************
 * File:        STRINP.C
 * Description: Three functions for input from the keyboard on MS-DOS
 *              computers. These routines are meant to be used with Borland
 *              Turbo C++ or compatible compilers, but can possibly be used
 *              with other compilers as well.
 * Author:      Rutger van Bergen
 *
 * This code may be distributed and altered free of charge.
 * The author can not be held responsible for any damage that may occur by
 * the use of (parts of) this code, altered or not, even if the author has
 * been informed of the possibility of such damage.
 * If any changes to this code are made and the code is spread with those
 * changes, you are kindly requested to report the changes made to the
 * original author.
 *
 * (C) Copyright 1996, Rutger van Bergen
 ***************************************************************************/

/***************************************************************************
 * Includes necessary for functions to work
 ***************************************************************************/
#include <ctype.h>
#include <varargs.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include "lib.h"
#include "conio.h"

/***************************************************************************
 * #defines of constants used by the functions
 ***************************************************************************/

/* Value used to indicate Escape keypress */
#define L_ESC     -2
/* Value used to indicate Enter keypress */
#define L_ENTER    5
/* Value used to indicate arrow up keypress */
#define L_UP       8
/* Value used to indicate arrow down keypress */
#define L_DOWN     2
/* Value used to indicate Tab keypress */
#define L_TAB      6
/* Value used to indicate Shift Tab keypress */
#define L_STAB     4
/* Character that will be written to screen in front of the actual input
    string if any the functions is used.
    Set to 0 to prevent a leading character being written. */
#define L_INPSTART 0
/* Character that will be written to screen behind the actual input string if
    any of the functions is used.
    Set to 0 to prevent a closing character being written. */
#define L_INPEND   0
/* Value that determines the settings for Insert for all functions.
    == 0: Insert off at start, use block cursor to indicate Insert on.
    == 1: Insert on at start, use block cursor to indicate Insert on.
    == 2: Insert off at start, use block cursor to indicate Insert off.
    == 3: Insert on at start, use block cursor to indicate Insert off. */
#define L_INSFLAG  3

/***************************************************************************
 * Definitions of the functions
 ***************************************************************************/

/*=========================================================================*
 * function: int agetchar(const char *allowed)
 * description: gets a character from the keyboard. It only accepts
 *              characters that are in the string allowed points to.
 * parameters: allowed: pointer to string that contains valid characters.
 *                      Characters that are not in this string will be
 *                      ignored.
 * return: the character that is entered, or L_ESC if Escape is pressed.
 * remarks: 1. before return, the function sets the cursor position to what it
 *             was before the function was called.
 *          2. Unlike the standard getchar function, agetchar waits for Enter
 *             to be pressed.
 *
 * example: y_or_n = agetchar("yYnN");
 *=========================================================================*/
int agetchar(allowed)
char *allowed;
{
    char input;

    do 
    {
        if (ascanf(0, 1, allowed, "%c", &input) == L_ESC)
            input = L_ESC;
    }
    while (input == ' ');
    return input;
}

/*=========================================================================*
 * function: int ascanf(int chckinp, int length, const char *allowed,
 * 			            const char *frmstr, ...)
 * description: extended version of scanf. Allows you to specify how many
 *              and which characters can be used for input.
 * parameters:  chckinp: 0: don't check if input was succesful, always exit
 *                           after Enter is pressed.
 *                       != 0: check if input was succesful. If not, repeat
 *                             the input process until it is.
 *              length: number of characters that can be entered during input.
 *              allowed: pointer to string that contains valid input-
 *                       characters. Characters that are not in this string
 *                       will be ignored.
 *              frmstr: format string that specifies the types of variables
 *                      to which the given input must be written, equal to the
 *                      format string of for instance scanf.
 *              ...: parameters to which the input has to be written, similar
 *                   to those of scanf.
 * return: L_ESC: Escape was pressed during input.
 *         EOF: error occured during writing input to parameters.
 *         other: number of input fields scanned.
 * remarks: 1. the function does not check if the input is too long for one
 *             line. If you make length so great that the last part of the
 *             input field runs off the screen line the results are undefined,
 *             but messy.
 *          2. before return, the function sets the cursor position to what
 *             it was before the function was called.
 *
 * example: ascanf(1, 25, "1234567890.", "%d %f", &intvar, &floatvar);
 *=========================================================================*/
int ascanf(chckinp, length, allowed, frmstr, va_alist)
int chckinp, length;
char *allowed, *frmstr;
va_dcl
{
    va_list argp;
    char *inpstr;
    int toret;

    inpstr = (char *) calloc(length + 1, sizeof(char));
    do 
    {
        memset(inpstr, ' ', length);
        if (!((toret = strinp(allowed, inpstr, wherex(), wherey(), 0, 1, 0)) == L_ESC)) 
        {
            va_start(argp);
            toret = vsscanf(inpstr, frmstr, argp);
            va_end(argp);
        }
    }
    while (chckinp && (toret == EOF || !toret));
    free(inpstr);
    return toret;
}

/*=========================================================================*
 * function: int strinp (const char *allowed, char *input, int inpx,
 *								 int inpy, int caps, int esc, int curm)
 * description: routine to read a string from stdin. It accepts all 'standard'
 *              editing keys (arrow right & left, BS, DEL, Home, End, INS).
 *              The characters that are valid for input can be specified,
 *              they can be auto-converted to upper- or lowercase and it is
 *              possible to make strinp return if arrow up, arrow down, Tab
 *              or Shift-Tab are pressed.
 * parameters:  allowed: pointer to string that contains valid input-
 *                       characters. Characters that are not in this string
 *                       will be ignored.
 *              input: pointer to string that has to be edited. Input is
 *                     started using this string as a basis. Any characters
 *                     that are entered are immediately processed into this
 *                     string.
 *              inpx: x-position where the first character of the input field
 *                    will be placed.
 *              inpy: y-position where the first character of the input field
 *                    will be placed.
 *              caps: < 0: all characters received from keyboard will be auto-
 *                         converted to lower case.
 *                    == 0: no converting will take place.
 *                    > 0: all characters received from keyboard will be auto-
 *                         converted to upper case.
 *              esc: == 0: Escape-keypresses will be ignored.
 *                   != 0: strinp will return on Escape-keypress.
 *              curm: == 0: strikes of Arrow up & down, Tab and Shift-Tab will
 *                          be ignored.
 *                    != 0: strikes of Arrow up & down, Tab and Shift-Tab will
 *                          make strinp return.
 * return: L_ESC: Escape was pressed during input and esc != 0.
 *         L_UP: Arrow up was pressed during input and curm != 0.
 *         L_DOWN: Arrow down was pressed during input and curm != 0.
 *         L_TAB: Tab was pressed during input and curm != 0.
 *         L_STAB: Shift-Tab was pressed during input and curm != 0.
 *         L_ENTER: strinp returned after Enter being pressed.
 * remarks: 1. the function does not check if the input is too long for one
 *             line. If you make length so great that the last part of the
 *             input field runs off the screen line the results are undefined,
 *             but messy.
 *          2. before return, the function sets the cursor position to what
 *             it was before the function was called.
 *
 * example: whyreturn = strinp("1234567890.", myinp, 10, 12, 0, 1, 1);
 *=========================================================================*/
int strinp (allowed, input, inpx, inpy, caps, esc, curm)
char *allowed, *input;
int inpx, inpy, caps, esc, curm;
{
    int ins = 1, ilen, ipos = 0, toret = 0, curx, cury, ichar;
    bool refresh_pending = TRUE;

    ilen = strlen(input) - 1;
    curx = wherex();
    cury = wherey();

    gotoxy(inpx, inpy);

    if (L_INPSTART) {
        putch(L_INPSTART);
        inpx++;
    }
    cputs(input);
    if (L_INPEND)
        putch(L_INPEND);

    do 
    {
        if (refresh_pending)
        {
            gotoxy(inpx + ipos, inpy);
            wrefresh(mainscr);
            refresh_pending = FALSE;
        }

        switch (ichar = getkeypress()) 
        {
        case KEY_LEFT:
            if (ipos) 
            { 
                ipos--;
                refresh_pending = TRUE;
            }
            break;

        case KEY_RIGHT:
            if (ipos < ilen) 
            {
                ipos++;
                refresh_pending = TRUE;
            }
            break;

        case KEY_UP:
            if (curm) toret = L_UP;
            break;

        case KEY_DOWN: 
            if (curm) toret = L_DOWN;
            break;

        case KEY_HOME: 
            ipos = 0;
            refresh_pending = TRUE;
            break;

        case KEY_END: 
            ipos = ilen;
            if (input[ipos] == ' ')
                while (ipos && input[ipos - 1] == ' ')
                    ipos--;

            refresh_pending = TRUE;
            break;

        case KEY_INSERT:
            ins = ins ? 0 : 1;
            break;

        case KEY_DELETE:
            memshift(input + ipos, input + ipos + 1, ilen - ipos);
            input[ilen] = ' ';
            cputs(input + ipos);
            refresh_pending = TRUE;
            break;

        case 127: /* Backspace */
            if (ipos)
            {  
                memshift(input + ipos - 1, input + ipos, ilen - ipos + 1);
                input[ilen] = ' ';
                gotoxy(inpx + --ipos, inpy);
                cputs(input + ipos);
                refresh_pending = TRUE;
            }
            break;

        case 9: /* Tab */
            if (curm) toret = L_TAB;
            break;

        case KEY_STAB: 
            if (curm) toret = L_STAB;
            break;
    
        /* I'm adding both because I *know* I've seen both, even though now it
           always seems to be 13... */
        case 10: /* Enter */
        case 13:
            toret = L_ENTER;
            break;

        case KEY_ESCAPE: /* Escape */
            if (esc) toret = L_ESC;
            break;

        default:
            if (ichar < 0)
                break;

            if (caps > 0 && islower(ichar))
                ichar = toupper(ichar);
            else if (caps < 0 && isupper(ichar))
                ichar = tolower(ichar);

            if (strchr(allowed, ichar)) 
            {
                putch(ichar);
                wrefresh(mainscr);

                if (ins) 
                {
                    memshift(input + ipos + 1, input + ipos, ilen - ipos);
                    cputs(input + ipos + 1);
                }

                input[ipos] = ichar;

                if (ipos < ilen)
                    ipos++;

                refresh_pending = TRUE;
            }
            break;
        }
    }
    while (!toret);

    return toret;
}

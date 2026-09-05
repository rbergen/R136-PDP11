/* Helper functions to load program data (basically texts) from files.
   Assume all of this to be horribly thread-unsafe! */

#include <stdio.h>
#include <strings.h>
#include "lib.h"
#include "conio.h"

char *language = "nl";

#define SINGLE_LINE_LENGTH  100
#define ROOM_TEXT_LENGTH    200

/* File numbers */

#define ITEM_NAMES			0
#define ITEM_DESCRIPTIONS	1

static char single_line_text[SINGLE_LINE_LENGTH];
static char room_text[ROOM_TEXT_LENGTH];

char *replace_char(str, find, replace)
char *str, find, replace;
{
    char *current_pos = strchr(str, find);

    while (current_pos)
    {
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }

    return str;
}

#define put_newlines(str) replace_char((str), '_', '\n')

FILE *OpenDataFile(letter, number)
char letter, number;
{
    char composed_path[64];
    char message[96];
    FILE *fp;

    sprintf(composed_path, "data/%s/%c%d", language, letter, number);

    fp = fopen(composed_path, "r");
    if (fp == NULL)
    {
        /* Careful: this can happen before mainscr exists, so don't use cputs(). */
        sprintf(message, "Error opening file %s!\n", composed_path);
        PrintError(message);
    }

    return fp;
}

void PrintLivingStatus(id, status)
char id, status;
{
    FILE *fp;
    char line[100];
    char *result;
    bool printed = FALSE;

    fp = OpenDataFile('l', id);
    if (fp == NULL)
        return;

    /* Skip lines until we find the file section for our status */
    do 
        result = fgets(line, 100, fp);
    while (result != NULL && (strlen(line) < 3 || (line[0] != 'K' || line[1] != 'J' || line[2] != 'n' - status)));

    /* This (EOF) can happen if the text file contains no text for our status */
    if (result == NULL)
    {
        fclose(fp);
        return;
    }

    /* Read and print the status text for our status, until we hit the next status marker */
    while (fgets(line, 100, fp) && (strlen(line) < 2 || (line[0] != 'K' || line[1] != 'J'))) 
    {
        fuzzle(line);
        cputs(line);
        printed = TRUE;
    }

    /* Only add another newline if we actually printed any status text */
    if (printed)
        putch('\n');

    fclose(fp);
}

char *GetSingleLineText(letter, number, line, add_newlines)
char letter, number, line;
bool add_newlines;
{
    int i, length;
    FILE *fp;
    char *result = NULL;

    /* Never hand a NULL back: every caller feeds this straight to cputs(). */
    single_line_text[0] = 0;

    fp = OpenDataFile(letter, number);
    if (fp == NULL)
        return single_line_text;

    for (i = 0; i <= line; i++)
    {
        result = fgets(single_line_text, SINGLE_LINE_LENGTH, fp);
        if (result == NULL)
            break;
    }

    fclose(fp);

    if (result == NULL)
    {
        single_line_text[0] = 0;
        return single_line_text;
    }

    /* Only strip an actual newline: the last line of a file may not have one */
    length = (int)strlen(single_line_text);
    if (length > 0 && single_line_text[length - 1] == '\n')
        single_line_text[length - 1] = 0;

    fuzzle(single_line_text);

    return add_newlines ? put_newlines(single_line_text) : single_line_text;
}

void GetRoomText(number, name, description)
char number, **name, **description;
{
    int i;
    FILE *fp;
    char filenumber, *semicolon;

    filenumber = number / 20;    /* We keep info for 20 rooms in each file */
    number %= 20;

    room_text[0] = 0;
    *name = room_text;
    *description = NULL;

    fp = OpenDataFile('r', filenumber);
    if (fp == NULL)
        return;

    for (i = 0; i <= number && fgets(room_text, ROOM_TEXT_LENGTH, fp); i++);

    fclose(fp);

    fuzzle(room_text);

    semicolon = strchr(room_text, ';');
    if (semicolon == NULL)
        return;

    *description = semicolon + 1;
    *semicolon = 0;

    put_newlines(*description);
}

int LoadStrings(string_array, count, letter, number, add_newlines)
char **string_array;
int count;
char letter, number;
bool add_newlines;
{
    int i, string_length, lines_read;
    FILE *fp;
    char line[100];

    for (i = 0; i < count; i++)
        string_array[i] = NULL;

    fp = OpenDataFile(letter, number);
    if (fp == NULL)
        lines_read = 0;
    else
    {
        for (i = 0; i < count && fgets(line, 100, fp); i++)
        {
            string_length = (int)strlen(line);

            /* We don't want the newline at the end of the string, but the last
               line of a file doesn't necessarily have one. */
            if (string_length > 0 && line[string_length - 1] == '\n')
                string_length--;

            string_array[i] = (char *)malloc(string_length + 1);
            if (string_array[i] == NULL)
                break;

            memcpy(string_array[i], line, string_length);
            string_array[i][string_length] = 0;
            fuzzle(string_array[i]);
        }

        lines_read = i;

        if (add_newlines)
            for (i = 0; i < lines_read; i++)
                put_newlines(string_array[i]);

        fclose(fp);
    }

    /* Anything we couldn't read becomes an empty string, so that printing it
       is harmless and freeing it is still valid. */
    for (i = lines_read; i < count; i++)
    {
        string_array[i] = (char *)malloc(1);
        if (string_array[i] != NULL)
            string_array[i][0] = 0;
    }

    return lines_read;
}

void PrintFile(letter, number, centered)
char letter, number;
bool centered;
{
    FILE *fp;
    char line[100];
    char *result;

    fp = OpenDataFile(letter, number);
    if (fp == NULL)
        return;

    while (fgets(line, 100, fp))
    {
        fuzzle(line);
        if (line[0] == 0); /* Ignore the last line without the newline*/
        else if (line[0] == '\n')
            putch('\n');
        /* If the first character isn't a newline then we have at least one character, 
           a newline and \0. */
        else if (line[0] == 'B' && line[1] == 'R' && line[2] == '\n')
        {
            getch();
            clrscr();
        }
        else if (centered)
            PrintCentered(line);
        else
            cputs(line);
    }

    fclose(fp);
}
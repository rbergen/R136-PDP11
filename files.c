/* Helper functions to load program data (basically texts) from files.
   Assume all of this to be horribly thread-unsafe! */

#include <stdio.h>
#include <strings.h>
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

char* defuzzle(str)
char *str;
{
    for (; *str && *str != '\n'; str++)
        *str = 158 - *str;

    return str;
}

#define put_newlines(str) replace_char((str), '_', '\n')

FILE *OpenDataFile(letter, number)
char letter, number;
{
    char composed_path[12];
    FILE *fp;

    sprintf(composed_path, "data/%s/%c%d", language, letter, number);

    fp = fopen(composed_path, "r");
    if (fp == NULL)
        wprintw(mainscr, "Error opening file %s!\n", composed_path);

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
    while (fgets(line, 100, fp) != NULL && (strlen(line) < 2 || (line[0] != 'K' || line[1] != 'J'))) 
    {
        defuzzle(line);
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
    int i;
    FILE *fp;

    fp = OpenDataFile(letter, number);
    if (fp == NULL)
        return NULL;

    for (i = 0; i <= line && fgets(single_line_text, SINGLE_LINE_LENGTH, fp) != NULL; i++);

    fclose(fp);

    single_line_text[strlen(single_line_text) - 1] = 0;
    defuzzle(single_line_text);

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

    fp = OpenDataFile('r', filenumber);

    for (i = 0; i <= number && fgets(room_text, ROOM_TEXT_LENGTH, fp) != NULL; i++);

    fclose(fp);

    defuzzle(room_text);

    *name = room_text;

    semicolon = strchr(room_text, ';');
    if (semicolon == NULL)
    {
        *description = NULL;
        return;
    }

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

    fp = OpenDataFile(letter, number);
    if (fp == NULL) 
        return;

    for (i = 0; i < count && fgets(line, 100, fp) != NULL; i++)
    {
        string_length = strlen(line);
        string_array[i] = (char *)malloc(string_length);
        /* We don't want the newline at the end of the string */
        memcpy(string_array[i], line, string_length - 1);
        string_array[i][string_length - 1] = 0;
        defuzzle(string_array[i]);
    }

    lines_read = i;

    if (add_newlines) 
        for (i = 0; i < count; i++)
            put_newlines(string_array[i]);

    fclose(fp);

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

    while (fgets(line, 100, fp) != NULL)
    {
        defuzzle(line);
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
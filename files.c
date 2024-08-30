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

char *replace_char(char *str, char find, char replace)
char *str, find, replace
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

    fp = OpenDataFile('l', id);
    if (fp == NULL)
        return;

    /* Skip lines until we find the file section for our status */
    while (fgets(line, 100, fp) != NULL && (line[0] != 'S' || cline[1] != 'T' || line[2] != status + '0'));

    /* Read and print the status text for our status, until we hit the next status marker */
    while (fgets(line, 100, fp) != NULL && (line[0] != 'S' || cline[1] != 'T'))
        cputs(line);

    putch('\n');

    fclose(fp);
}

char *GetSingleLineText(letter, number, line, add_newlines)
char letter, number, line;
bool add_newlines;
{
    FILE *fp;

    fp = OpenDataFile(letter, number);
    if (fp == NULL)
        return NULL;

    for (i = 0; i <= line && fgets(single_line_text, SINGLE_LINE_LENGTH, fp) != NULL; i++);

    fclose(fp);

    single_line_text[strlen(single_line_text) - 1] = 0;

    return add_newlines ? put_newlines(single_line_text) : single_line_text;
}

void LoadItemName(number, name)
char number, **name;
{
    char *loaded_name;

    loaded_name = GetSingleLineText('i', ITEM_NAMES, number, false);
    *name = (char *)malloc(strlen(loaded_name) + 1);
    strcpy(*name, loaded_name);
}

void GetRoomText(number, name, description)
char number, **name, **description;
{
    FILE *fp;
    char filenumber;
    char *semicolon;

    filenumber = number / 20;    /* We keep info for 20 rooms in each file */
    number %= 20;

    fp = OpenDataFile('r', filenumber);

    for (i = 0; i <= number && fgets(room_text, ROOM_TEXT_LENGTH, fp) != NULL; i++);

    fclose(fp);

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
    int i;
    FILE *fp;
    char[100] line;

    fp = OpenDataFile('p', 0);
    if (fp == NULL) 
        return;

    for (i = 0; i < count && fgets(line, 100, fp) != NULL; i++)
    {
        string_array[i] = (char *)malloc(strlen(line) + 1);
        strcpy(string_array[i], line);
        if (add_newlines)
            put_newlines(string_array[i]);
    }

    fclose(fp);

    return i;
}

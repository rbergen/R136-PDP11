#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include "r136.h"

/* init.c */
void Initialize();
void Deinitialize();

/* loadsave.c */
void SaveStatus();
char LoadStatus();

/* status.c */
void RoomStatus();
char BeastStatus();

/* use.c */
bool DoAction();

#define START_MESSAGE 0
#define SPLASH_SCREEN 1

extern char *language;

void PrintHelp(progname)
char *progname;
{
    DIR *d;
    struct direct *entry;
    struct stat st;
    char langpath[256];

    printf("Usage: %s -l <language>\n", progname);
    printf("Options:\n");
    printf("  -l <language>  Set the language to use. Available languages (default is %s):\n", language);

    d = opendir("data");
    if (d) 
    {
        while (entry = readdir(d))
        {
            sprintf(langpath, "data/%s", entry->d_name);

            if (!stat(langpath, &st) && S_ISDIR(st.st_mode) 
                && strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
            {
                printf("                 * %s\n", entry->d_name);
            }
        }

        closedir(d);
    }

    printf("  -h, -?         Display this help\n\n");
}

bool ParseArgs(argc, argv)
int argc;
char *argv[];
{
    struct stat st;
    int opt;
    char langpath[256];

    while ((opt = getopt(argc, argv, "l:h?")) != -1) 
    {
        if (opt == 'l')
        {
            sprintf(langpath, "data/%s", optarg);

            if (!stat(langpath, &st) && S_ISDIR(st.st_mode)) 
            {

                language = (char *)malloc(strlen(optarg) + 1);
                strcpy(language, optarg);
                return TRUE;
            }
        }

        PrintHelp(argv[0]);
        return FALSE;
    }

    return TRUE;
}

int main(argc, argv)
int argc;
char *argv[];
{
    Progdata progdata;

    if (!ParseArgs(argc, argv))
        return 0;

    Initialize(&progdata);

    PrintFile('s', SPLASH_SCREEN, TRUE);
    
    if (!LoadStatus(&progdata)) 
    {
        clrscr();
        PrintFile('s', START_MESSAGE, FALSE);
    }

    while (TRUE)
    {
        RoomStatus(&progdata);
        if (!BeastStatus(&progdata) || !DoAction(&progdata))
            break;
    }

    SaveStatus(&progdata);

    getch();

    Deinitialize(&progdata);

    return 0;
}

void ForceExit(progdata)
Progdata *progdata;
{
    getch();

    unlink(DATA_FILE);

    Deinitialize(&progdata);

    exit(0);
}


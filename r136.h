#include <ctype.h>
#include <varargs.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include "conio.h"

/*
  Defines
*/

/* Flag values */

#define NO_COMMAND      -1
#define NO_ROOM         -1
#define OWNED           -2
#define NO_ITEM         -1
#define AMBIGUOUS_ITEM  -2
#define INFINITE_POINTS -1
#define NOTHING          1

#define PRELOADED_STRING_COUNT  28
#define COMMAND_COUNT           15
#define ROOM_COUNT              80
#define LIVING_COUNT            21
#define ITEM_COUNT              25
#define MAX_OWNED_ITEMS         10

#define BIG_WOUND    4

/*
  Structures
*/

typedef struct
{
    char connect[6];
} Room;

typedef struct
{
    char room;
    char strike;
    char status;
} Living;

typedef struct
{
    char *name;
    char room;
    char useableon;
} Item;

typedef struct
{
    char paperpos;
    char curroom;
    char lifepoints;
    bool lamp;
    char lamppoints;
} Status;

typedef struct
{
    Room rooms[ROOM_COUNT];
    Living living[LIVING_COUNT];
    Item items[ITEM_COUNT];
    char owneditems[MAX_OWNED_ITEMS];
    char paperroute[6];
    Status status;
    char *strings[PRELOADED_STRING_COUNT];
    char *commands[COMMAND_COUNT];
} Progdata;

typedef struct
{
    char command;
    char object1;
    char object2;
    bool error;
} Parsedata;

#define rnd(x)      (random() % (x))

extern char *language;

/* strinp.c */
int agetchar();
int ascanf();
int strinp ();

/* status.c */
void ApplySimmeringForest();
bool IsRoomDark();

/* main.c */
void ForceExit();

/* files.c */
char *GetDataPath();
void PrintLivingStatus();
char *GetSingleLineText();
void GetRoomText();
int LoadStrings();
void LoadItemName();
void PrintFile()



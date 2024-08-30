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

#define NO_COMMAND		-1
#define NO_ROOM			-1
#define OWNED			-2
#define NO_ITEM			-1
#define AMBIGUOUS_ITEM 	-2
#define INFINITE_POINTS	-1
#define NOTHING			-1

/* Commands */

#define EAST		0
#define WEST		1
#define NORTH		2
#define SOUTH		3
#define UP			4
#define DOWN		5
#define GEBRUIK		6
#define COMBINEER	7
#define PAK			8
#define LEG			9
#define BEKIJK		10
#define AFWACHTEN	11
#define EINDE		12
#define STATUS		13
#define HELP		14

#define ROOM_COUNT		80
#define MAX_OWNED_ITEMS	10

/* Living objects */

#define HELLEHOND		0
#define RODETROL		1
#define PLANT			2
#define GNOE			3
#define DRAAK			4
#define GEZWEL			5
#define DEUR			6
#define STEMMEN			7
#define BARBECUE		8
#define BOOM			9
#define DIAMANT			10
#define COMPUTER		11
#define DRAKEKOP		12
#define LAVA			13
#define VACUUM			14
#define PAPIER			15
#define NOORDMOERAS		16
#define MIDDENMOERAS	17
#define ZUIDMOERAS		18
#define MISTGROT		19
#define TELEPORT		20
#define LIVING_COUNT	21

/* Items */

#define HONDVLEES		0
#define HITTEPAK		1
#define GROENKRISTAL	2
#define ZWAARD			3
#define BOT				4
#define DISKETTE		5
#define HASJ			6
#define ROODKRISTAL		7
#define SLAAPMUTS		8
#define BOM				9
#define ZAKLAMP			10
#define VERBAND			11
#define VLAMMENWERPER	12
#define KOOKBOEK		13
#define TNT				14
#define GASPATROON		15
#define GIFTIGVLEES		16
#define ONTSTEKING		17
#define BATTERIJEN		18
#define GASMASKER		19
#define PAPIERITEM		20
#define BOEKJE			21
#define BLAUWKRISTAL	22
#define KOEKJE			23
#define GASGRANAAT		24
#define ITEM_COUNT		25

#define ON_ITEM(item)	(-(item + 2))

/* Preloaded strings. 0 to 5 are taken by the directions of navigation. */

#define YOU_CAN_GO_TO			6
#define AND						7
#define YOU_ARE_HERE			8
#define TOO_DARK_TO_SEE			9
#define ITEM_HERE				10
#define ITEMS_HERE				11
#define PRELOADED_STRING_COUNT	12

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
} Progdata;

typedef struct
{
	char command;
	char object1;
	char object2;
	bool error;
} Parsedata;

#define rnd(x) (random() % (x))

extern char *language;

// strinp.c
int agetchar();
int ascanf();
int strinp ();

// init.c
bool Initialize();

// status.c
void ApplySimmeringForest();

// main.c
void ForceExit();

// files.c
char *GetDataPath();
void PrintLivingStatus();
char *GetSingleLineText();
void GetRoomText();
int LoadStrings();
void LoadItemName();


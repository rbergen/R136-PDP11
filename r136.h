#include <conio.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>

/*
  structures
*/

struct Room
{
	char connect[6];
	char *name;
	char *descript;
};

struct Living
{
	char room;
	char strike;
	char status;
};

struct Item
{
	char *name;
	char *descript;
	char room;
	char useableon;
};

struct Status
{
	char paperpos;
	char curroom;
	char lifepoints;
	bool lamp;
	char lamppoints;
};

struct Progdata
{
	Room rooms[80];
	Living living[21];
	Item items[25];
	char owneditems[10];
	char paperroute[6];
	Status status;
};

struct Parsedata
{
	char command;
	char object1;
	char object2;
	bool error;
};

/*
  defines
*/

#define NO_COMMAND	   -1
#define NO_ROOM		   -1
#define OWNED				-2
#define NO_ITEM         -1
#define AMBIGUOUS_ITEM  -2
#define INFINITE_POINTS -1
#define NOTHING			-1

#define EAST		0
#define WEST		1
#define NORTH		2
#define SOUTH		3
#define UP			4
#define DOWN		5
#define GEBRUIK	6
#define COMBINEER	7
#define PAK			8
#define LEG			9
#define BEKIJK		10
#define AFWACHTEN	11
#define EINDE		12
#define STATUS		13
#define HELP		14

#define HELLEHOND		0
#define RODETROL		1
#define PLANT			2
#define GNOE			3
#define DRAAK			4
#define GEZWEL			5
#define DEUR			6
#define STEMMEN		7
#define BARBECUE		8
#define BOOM			9
#define DIAMANT		10
#define COMPUTER		11
#define DRAKEKOP		12
#define LAVA			13
#define VACUUM			14
#define PAPIER			15
#define NOORDMOERAS	16
#define MIDDENMOERAS	17
#define ZUIDMOERAS	18
#define MISTGROT		19
#define TELEPORT		20

#define HONDVLEES			0
#define HITTEPAK			1
#define GROENKRISTAL		2
#define ZWAARD				3
#define BOT					4
#define DISKETTE			5
#define HASJ				6
#define ROODKRISTAL		7
#define SLAAPMUTS			8
#define BOM					9
#define ZAKLAMP			10
#define VERBAND			11
#define VLAMMENWERPER	12
#define KOOKBOEK			13
#define TNT					14
#define GASPATROON		15
#define GIFTIGVLEES		16
#define ONTSTEKING		17
#define BATTERIJEN		18
#define GASMASKER			19
#define PAPIERITEM		20
#define BOEKJE				21
#define BLAUWKRISTAL		22
#define KOEKJE				23
#define GASGRANAAT		24

#define ON_ITEM(item) (-(item + 2))

#define E_GRAVE "\x82"
#define E_TREMA "\x89"

// strinp.cpp
int agetchar(const char *allowed);
int ascanf(int chckinp, int length, const char *allowed, const char *frmstr, ...);
int strinp (const char *allowed, char *input, int inpx, int inpy, int caps, int esc, int curm);

// loadsave.cpp
void SaveStatus(Progdata &progdata);
bool LoadStatus(Progdata &progdata);

// intro.cpp
void RunIntro(void);
void ShowSplashScreen(void);
void ShowStartMessage(void);

// init.cpp
bool Initialize(Progdata &progdata);

// use.cpp
bool DoAction(Progdata &progdata);

// main.cpp
void PrintCentered(const char *text, int offset = 0);
void ForceExit(void);

// status.cpp
void RoomStatus(Progdata &progdata);
bool BeastStatus(Progdata &progdata);
void ApplySimmeringForest(Progdata &progdata);



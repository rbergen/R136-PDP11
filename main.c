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

int main()
{
	Progdata progdata;

	Initialize(&progdata);

	PrintFile('s', SPLASH_SCREEN, TRUE);
	
	if (!LoadStatus(&progdata))
		PrintFile('s', START_MESSAGE, FALSE);

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


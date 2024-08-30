#include "r136.h"

// loadsave.c
void SaveStatus();
char LoadStatus();

// intro.c
void ShowSplashScreen();
void ShowStartMessage();

// use.c
bool DoAction();

// status.c
void RoomStatus();
char BeastStatus();

int main(void)
{
	Progdata progdata;

	Initialize(&progdata);

	ShowSplashScreen();

	if (!LoadStatus(&progdata))
		ShowStartMessage();

	while (true)
	{
		RoomStatus(&progdata);
		if (BeastStatus(&progdata))
			if (!DoAction(&progdata))
				break;
	}

	SaveStatus(&progdata);

	getch();

	delwin(mainscr);
	clrscr();
	endwin();

	return 0;
}

void ForceExit(void)
{
	char fpath[255];

	getch();

	memset(fpath, 0, 255);
	strcat(strncpy(fpath, _argv[0], int(strrchr(_argv[0], '\\') - _argv[0]) + 1), "data.rip");
	unlink(fpath);

	delwin(mainscr);
	clrscr();
	endwin();

	exit(0);
}


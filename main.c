#include "r136.h"

int main(void)
{
   RunIntro();

	Progdata progdata;

	Initialize(progdata);

	ShowSplashScreen();

	if (!LoadStatus(progdata))
		ShowStartMessage();

	while (true)
	{
		RoomStatus(progdata);
		if (BeastStatus(progdata))
			if (!DoAction(progdata))
				break;
	}

	SaveStatus(progdata);

	getch();

	window(1, 1, 80, 25);
	clrscr();

	return 0;
}

void PrintCentered(const char *text, int offset)
{
	int numspaces = (80 - strlen(text) - offset) / 2;
	char* spaces = (char *)malloc(numspaces + 1);

   memset(spaces, ' ', numspaces);
	spaces[numspaces] = 0;
   cputs(spaces);
	free(spaces);

   cputs(text);
}

void ForceExit(void)
{
	char fpath[255];

	getch();

	memset(fpath, 0, 255);
	strcat(strncpy(fpath, _argv[0], int(strrchr(_argv[0], '\\') - _argv[0]) + 1), "data.rip");
	remove(fpath);

	window(1, 1, 80, 25);
	clrscr();

	exit(0);
}


#include "r136.h"

void ReportSaveFailure(FILE *fp, const char *fpath)
{
 	textcolor(YELLOW);
	cputs("Fout bij wegschrijven status.");
   textcolor(LIGHTGRAY);
   cputs("\r\n\r\nStatus niet opgeslagen!\r\n");
	fclose(fp);
	remove(fpath);
}

void SaveStatus(Progdata &progdata)
{
	FILE *fp;
	char fpath[255];
	int i;

	memset(fpath, 0, 255);

	strcat(strncpy(fpath, _argv[0], int(strrchr(_argv[0], '\\') - _argv[0]) + 1), "data.rip");

   textcolor(YELLOW);
	cputs("\r\n\r\nWil je je huidige status opslaan? ");
	if (tolower(agetchar("jJnN")) != 'j')
	{
		cprintf("\r\n");
		return;
	}

	while ((fp = fopen(fpath, "wb")) == NULL)
	{
   	textcolor(YELLOW);
      cputs("\r\n\r\nKon het save-bestand niet openen voor schrijven. Nogmaals proberen? ");
		if (tolower(agetchar("jJnN")) != 'j')
		{
			cputs("\r\n\r\nStatus niet opgeslagen!\r\n");
			remove(fpath);
			return;
		}
	}

	cprintf("\r\n");

	for (i = 0; i < 25; i++)
		if (fwrite(&progdata.items[i].room, sizeof(char), 1, fp) < 1)
		{
      	ReportSaveFailure(fp, fpath);
			return;
		}
	for (i = 0; i < 80; i++)
		if (fwrite(progdata.rooms[i].connect, sizeof(char) * 6, 1, fp) < 1)
		{
      	ReportSaveFailure(fp, fpath);
			return;
		}
	for (i = 0; i < 21; i++)
		if (fwrite(&progdata.living[i], sizeof(Living), 1, fp) < 1)
		{
      	ReportSaveFailure(fp, fpath);
			return;
		}
	if (fwrite(progdata.owneditems, sizeof(char), 10, fp) < 10)
	{
     	ReportSaveFailure(fp, fpath);
		return;
	}
	if (fwrite(&progdata.status, sizeof(Status), 1, fp) < 1)
	{
     	ReportSaveFailure(fp, fpath);
		return;
	}
	fclose(fp);
}

bool ReportLoadFailure(Progdata &progdata, FILE *fp, const char *fpath)
{
 	textcolor(YELLOW);
	cputs("Fout bij lezen status.");
   textcolor(LIGHTGRAY);
   cputs("\r\n\r\nJe start een nieuw spel. ");
	fclose(fp);
	remove(fpath);
   textcolor(YELLOW);
   cputs("Druk op een toets om te beginnen...");
	textcolor(LIGHTGRAY);
	getch();
   cputs("\r\n\r\n");
	Initialize(progdata);
	return false;
}

bool LoadStatus(Progdata &progdata)
{
	FILE *fp;
	char fpath[255];
	int i;

	memset(fpath, 0, 255);

	strcat(strncpy(fpath, _argv[0], int(strrchr(_argv[0], '\\') - _argv[0]) + 1), "data.rip");

	if ((fp = fopen(fpath, "rb")) == NULL)
	{
   	textcolor(YELLOW);
		PrintCentered("Druk op een toets om te beginnen...");
      textcolor(LIGHTGRAY);
		getch();
		return false;
	}

   textcolor(YELLOW);
	PrintCentered("Toets 1 voor een nieuw spel, 2 voor een gesaved spel: ", 1);
	if (agetchar("12") != '2')
	{
		fclose(fp);
		remove(fpath);
		return false;
	}
	cputs("\r\n");

	for (i = 0; i < 25; i++)
		if (fread(&progdata.items[i].room, sizeof(char), 1, fp) < 1)
      	ReportLoadFailure(progdata, fp, fpath);
	for (i = 0; i < 80; i++)
		if (fread(progdata.rooms[i].connect, sizeof(char) * 6, 1, fp) < 1)
      	ReportLoadFailure(progdata, fp, fpath);
	for (i = 0; i < 21; i++)
		if (fread(&progdata.living[i], sizeof(Living), 1, fp) < 1)
      	ReportLoadFailure(progdata, fp, fpath);
	if (fread(progdata.owneditems, sizeof(char), 10, fp) < 10)
   	ReportLoadFailure(progdata, fp, fpath);
	if (fread(&progdata.status, sizeof(Status), 1, fp) < 1)
      ReportLoadFailure(progdata, fp, fpath);
	cputs("\r\n");
	fclose(fp);

   if (progdata.living[BOOM].status == 2)
   	ApplySimmeringForest(progdata);

	return true;
}


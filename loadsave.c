#include "r136.h"

void ReportSaveFailure(fp, fpath)
FILE *fp;
char *fpath;
{
    cputs("Fout bij wegschrijven status.");
    cputs("\n\nStatus niet opgeslagen!\n");
    fclose(fp);
    unlink(fpath);
}

void SaveStatus(progdata)
Progdata *progdata;
{
    FILE *fp;
    char fpath[255];
    int i;

    memset(fpath, 0, 255);

    strcat(strncpy(fpath, _argv[0], int(strrchr(_argv[0], '\\') - _argv[0]) + 1), "data.rip");

    cputs("\n\nWil je je huidige status opslaan? ");
    if (tolower(agetchar("jJnN")) != 'j')
    {
        putch('\n');
        return;
    }

    while ((fp = fopen(fpath, "wb")) == NULL)
    {
        textcolor(YELLOW);
        cputs("\n\nKon het save-bestand niet openen voor schrijven. Nogmaals proberen? ");
        if (tolower(agetchar("jJnN")) != 'j')
        {
            cputs("\n\nStatus niet opgeslagen!\n");
            unlink(fpath);
            return;
        }
    }

    putch('\n');

    for (i = 0; i < 25; i++)
        if (fwrite(&(progdata->items[i].room), sizeof(char), 1, fp) < 1)
        {
            ReportSaveFailure(fp, fpath);
            return;
        }
    for (i = 0; i < 80; i++)
        if (fwrite(progdata->rooms[i].connect, sizeof(char) * 6, 1, fp) < 1)
        {
            ReportSaveFailure(fp, fpath);
            return;
        }
    for (i = 0; i < 21; i++)
        if (fwrite(&(progdata->living[i]), sizeof(Living), 1, fp) < 1)
        {
            ReportSaveFailure(fp, fpath);
            return;
        }
    if (fwrite(progdata->owneditems, sizeof(char), 10, fp) < 10)
    {
        ReportSaveFailure(fp, fpath);
        return;
    }
    if (fwrite(&(progdata->status), sizeof(Status), 1, fp) < 1)
    {
        ReportSaveFailure(fp, fpath);
        return;
    }
    fclose(fp);
}

bool ReportLoadFailure(progdata, fp, fpath)
Progdata *progdata;
FILE *fp;
char *fpath;
{
    cputs("Fout bij lezen status.");
    cputs("\n\nJe start een nieuw spel. ");
    fclose(fp);
    unlink(fpath);
    cputs("Druk op een toets om te beginnen...");
    getch();
    cputs("\n\n");
    Initialize(progdata);
    return false;
}

bool LoadStatus(progdata)
Progdata *progdata;
{
    FILE *fp;
    char fpath[255];
    int i;

    memset(fpath, 0, 255);

    strcat(strncpy(fpath, _argv[0], int(strrchr(_argv[0], '\\') - _argv[0]) + 1), "data.rip");

    if ((fp = fopen(fpath, "rb")) == NULL)
    {
        PrintCentered("Druk op een toets om te beginnen...");
        getch();
        return false;
    }

    PrintCentered("Toets 1 voor een nieuw spel, 2 voor een gesaved spel: ");
    if (agetchar("12") != '2')
    {
        fclose(fp);
        unlink(fpath);
        return false;
    }
    cputs("\n");

    for (i = 0; i < 25; i++)
        if (fread(&(progdata->items[i].room), sizeof(char), 1, fp) < 1)
            ReportLoadFailure(progdata, fp, fpath);
    for (i = 0; i < 80; i++)
        if (fread(progdata->rooms[i].connect, sizeof(char) * 6, 1, fp) < 1)
            ReportLoadFailure(progdata, fp, fpath);
    for (i = 0; i < 21; i++)
        if (fread(&(progdata->living[i]), sizeof(Living), 1, fp) < 1)
            ReportLoadFailure(progdata, fp, fpath);
    if (fread(progdata->owneditems, sizeof(char), 10, fp) < 10)
        ReportLoadFailure(progdata, fp, fpath);
    if (fread(&(progdata->status), sizeof(Status), 1, fp) < 1)
        ReportLoadFailure(progdata, fp, fpath);
    cputs("\n");
    fclose(fp);

   if (progdata->living[BOOM].status == 2)
        ApplySimmeringForest(progdata);

    return true;
}


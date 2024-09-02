#include "r136.h"
#include "living.h"
#include "ldstr.h"

void ReportSaveFailure(progdata, fp, fpath)
Progdata *progdata;
FILE *fp;
char *fpath;
{
    cputs(Str(ERROR_WRITING_DATA));
    cputs(Str(STATUS_NOT_SAVED));
    fclose(fp);
    unlink(fpath);
}

void SaveStatus(progdata)
Progdata *progdata;
{
    FILE *fp;
    int i;
    char *fpath = DATA_FILE;

    cputs(Str(WANT_TO_SAVE_GAME));
    if (!IsConfirmed(progdata))
    {
        putch('\n');
        unlink(fpath);
        return;
    }

    while ((fp = fopen(fpath, "w")) == NULL)
    {
        cputs(Str(COULDNT_OPEN_SAVE_FILE));
        if (!IsConfirmed(progdata))
        {
            cputs(Str(STATUS_NOT_SAVED));
            unlink(fpath);
            return;
        }
    }

    putch('\n');

    for (i = 0; i < ITEM_COUNT; i++)
        if (fwrite(&(progdata->items[i].room), sizeof(char), 1, fp) < 1)
        {
            ReportSaveFailure(progdata, fp, fpath);
            return;
        }
    for (i = 0; i < ROOM_COUNT; i++)
        if (fwrite(progdata->rooms[i].connect, sizeof(char) * 6, 1, fp) < 1)
        {
            ReportSaveFailure(progdata, fp, fpath);
            return;
        }
    for (i = 0; i < LIVING_COUNT; i++)
        if (fwrite(&(progdata->living[i]), sizeof(Living), 1, fp) < 1)
        {
            ReportSaveFailure(progdata, fp, fpath);
            return;
        }
    if (fwrite(progdata->owneditems, sizeof(char), 10, fp) < 10)
    {
        ReportSaveFailure(progdata, fp, fpath);
        return;
    }
    if (fwrite(&(progdata->status), sizeof(Status), 1, fp) < 1)
    {
        ReportSaveFailure(progdata, fp, fpath);
        return;
    }
    fclose(fp);
}

bool ReportLoadFailure(progdata, fp, fpath)
Progdata *progdata;
FILE *fp;
char *fpath;
{
    cputs(Str(ERROR_READING_DATA));
    fclose(fp);
    unlink(fpath);
    cputs(Str(PRESS_ANY_KEY));
    getch();
    cputs("\n\n");
    Initialize(progdata);
    return FALSE;
}

bool LoadStatus(progdata)
Progdata *progdata;
{
    FILE *fp;
    int i;
    char *fpath = DATA_FILE;

    if ((fp = fopen(fpath, "r")) == NULL)
    {
        PrintCentered(Str(PRESS_ANY_KEY));
        getch();
        return FALSE;
    }

    PrintCentered(Str(NEW_OR_SAVED_GAME));
    if (agetchar("12") != '2')
    {
        fclose(fp);
        unlink(fpath);
        return FALSE;
    }
    cputs("\n");

    for (i = 0; i < ITEM_COUNT; i++)
        if (fread(&(progdata->items[i].room), sizeof(char), 1, fp) < 1)
            ReportLoadFailure(progdata, fp, fpath);
    for (i = 0; i < ROOM_COUNT; i++)
        if (fread(progdata->rooms[i].connect, sizeof(char) * 6, 1, fp) < 1)
            ReportLoadFailure(progdata, fp, fpath);
    for (i = 0; i < LIVING_COUNT; i++)
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

    clrscr();

    return TRUE;
}


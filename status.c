#include "r136.h"
#include "items.h"
#include "living.h"
#include "ldstr.h"

void ShowDirString();
void ShowItems();
void HellehondStatus();
void RodeTrolStatus();
void PlantStatus();
void GnoeStatus();
void DraakStatus();
void GezwelStatus();
void BarbecueStatus();
void BoomStatus();
void DrakeKopStatus();
bool LavaStatus();

#define BIG_WOUND    4

bool IsRoomDark(roomnumber)
int roomnumber;
{
    return roomnumber >= 20   /* Caves are rooms >=20 and dark, except: */
        && roomnumber != 31   /* Fluorescent tube cave */
        && roomnumber != 48   /* Emptiness */
        && roomnumber != 49   /* Sand bank */
        && roomnumber != 53   /* Chasm */
        && roomnumber != 61;  /* Radioactive cave */
}

void RoomStatus(progdata)
Progdata *progdata;
{
    char *roomname;
    char *roomdescript;

    GetRoomText(progdata->status.curroom, &roomname, &roomdescript);

    wprintw(mainscr, progdata->strings[YOU_ARE_HERE], roomname);

    if (IsRoomDark(progdata->status.curroom) && !progdata->status.lamp)
        cputs(progdata->strings[TOO_DARK_TO_SEE]);
    else
    {
        if (progdata->rooms[progdata->status.curroom].descript)
            roomdescript = progdata->rooms[progdata->status.curroom].descript;

        if (roomdescript)
        {
            cputs(roomdescript);
            putch('\n');
        }
        ShowItems(progdata);
    }

    ShowDirString( );
    cputs("\n");
}

void ShowDirString(progdata)
Progdata *progdata;
{
    int count = 0;
    int i;
    char *connect = progdata->rooms[progdata->status.curroom].connect;

    for (i = 0; i < 6; i++)
        if (connect[i] != NO_ROOM)
            count++;

    if (!count)
        return;

    cputs(progdata->strings[YOU_CAN_GO_TO]);

    for (i = 0; i < 6; i++)
        if (connect[i] != NO_ROOM)
        {
            cputs(progdata->strings[i]);
            switch(--count)
            {
            case 0:
                break;
            case 1:
                cputs(progdata->strings[AND]);
                break;
            default:
                cputs(", ");
                break;
            }
        }

    cputs(".\n");
}

void ShowItems(progdata)
Progdata *progdata;
{
    int count = 0;
    int i;

    for (i = 0; i < 25; i++)
        if (progdata->items[i].room == progdata->status.curroom)
            count++;

    if (count)
    {
        cputs(progdata->strings[count > 1 ? ITEMS_HERE : ITEM_HERE]);

        for (i = 0; i < ITEM_COUNT; i++)
            if (progdata->items[i].room == progdata->status.curroom)
            {
                if (wherex() > 55)
                    putch('\n');
                cputs(progdata->items[i].name);

                switch(--count)
                {
                case 0:
                    break;
                case 1:
                    cputs(progdata->strings[AND]);
                    break;
                default:
                    cputs(", ");
                    break;
                }
            }

        cputs(".\n");
    }
}


bool BeastStatus(progdata)
Progdata *progdata;
{
    int beastfound = -1;
    int i;

    for (i = 0; i < 21 && beastfound == -1; i++)
        if (progdata->living[i].room == progdata->status.curroom)
            beastfound = i;

    if (beastfound == -1)
        return TRUE;

    PrintLivingStatus(beastfound, progdata->living[beastfound].status);

    switch (beastfound)
    {
    case HELLEHOND:
        HellehondStatus(progdata);
        break;

    case RODETROL:
        RodeTrolStatus(progdata);
        break;
    
    case PLANT:
        PlantStatus(progdata);
        break;
    
    case GNOE:
        GnoeStatus(progdata);
        break;
    
    case DRAAK:
        DraakStatus(progdata);
        break;
    
    case GEZWEL:
        GezwelStatus(progdata);
        break;
    
    case DEUR:
        if (progdata->living[DEUR].status == 1)
        {
            progdata->rooms[45].connect[NORTH] = 40;
            progdata->living[DEUR].status = 2;
        }
        break;
    
    case STEMMEN:
        if (!progdata->living[STEMMEN].status)
            progdata->living[STEMMEN].status = 1;
        break;
    
    case BARBECUE:
        BarbecueStatus(progdata);
        break;
    
    case BOOM:
        BoomStatus(progdata);
        break;
    
    case COMPUTER:
        if (!progdata->living[COMPUTER].status || progdata->living[COMPUTER].status == 2)
            progdata->living[COMPUTER].status++;
        break;
    
    case DRAKEKOP:
        DrakeKopStatus(progdata);
        break;
    
    case LAVA:
        return LavaStatus(progdata);
    
    case VACUUM:
        progdata->status.lifepoints -= BIG_WOUND;
        progdata->status.curroom = 76; /* Back one cave */
        return FALSE;
    
    case PAPIER:
        if (progdata->living[PAPIER].status == 1)
        {
            progdata->items[PAPIERITEM].room = 66;
            progdata->living[PAPIER].status = 2;
        }
        break;
    
    case NOORDMOERAS:
    case MIDDENMOERAS:
    case ZUIDMOERAS:
        switch (progdata->status.curroom)
        {
        case 3:
            progdata->status.curroom = 51;
            break;
        case 8:
            progdata->status.curroom = 30;
            break;
        case 18:
            progdata->status.curroom = 47;
            break;
        }
        return FALSE;
    
    case MISTGROT:
        switch(rnd(3))
        {
        case 0:
            progdata->status.curroom = 36;
            break;
        case 1:
            progdata->status.curroom = 32;
            break;
        case 2:
            progdata->status.curroom = 38;
            break;
        }
        return FALSE;
    
    case TELEPORT:
        progdata->status.curroom = 1;
        return FALSE;
    }
    return TRUE;
}

void HellehondStatus(progdata)
Progdata *progdata;
{
    switch (progdata->living[HELLEHOND].status)
    {
    case 1:
        progdata->status.lifepoints--;
        /* Deliberate fall-through! */
    case 0:
        progdata->living[HELLEHOND].status++;
        break;

    case 2:
        progdata->living[HELLEHOND].status = rnd(2) + 1;
        break;

    case 3:
        progdata->items[HONDVLEES].room = progdata->status.curroom;
        progdata->living[HELLEHOND].status = 4;
        break;
    }
}

void RodeTrolStatus(progdata)
Progdata *progdata;
{
    switch (progdata->living[RODETROL].status)
    {
    case 2:
        progdata->status.lifepoints--; 
        /* Deliberate fall-through! */
    case 0:
    case 1:
        progdata->living[RODETROL].status++;
        break;

    case 3:
        progdata->living[RODETROL].status = rnd(2) + 2;
        break;

    case 4:
        progdata->items[ROODKRISTAL].room = progdata->status.curroom;
        progdata->living[RODETROL].status = 5;
        break;
    }
}

void PlantStatus(progdata)
Progdata *progdata;
{
    switch (progdata->living[PLANT].status)
    {
    case 0:
    case 2:
        progdata->status.lifepoints--; 
        progdata->living[PLANT].status = rnd(2) + 1;
        break;
        progdata->living[PLANT].status = 2;
        break;
    case 3:
        progdata->rooms[25].connect[NORTH] = 20; /*	Maak verbinding met slijmgrot */
        /* Deliberate fall-through! */
    case 1:
        progdata->living[PLANT].status++;
        break;
    }
}

void GnoeStatus(progdata)
Progdata *progdata;
{
    switch (progdata->living[GNOE].status)
    {
    case 0:
    case 2:
        progdata->living[GNOE].status++;
        break;

    case 1:
        progdata->status.lifepoints--; 
        break;
    }
}

void DraakStatus(progdata)
Progdata *progdata;
{
    switch (progdata->living[DRAAK].status)
    {

    case 2:
        progdata->status.lifepoints--; 
        break;

    case 5:
        progdata->items[GASPATROON].room = progdata->status.curroom;
        /* Deliberate fall-through! */
    case 0:
    case 1:
    case 3:
        progdata->living[DRAAK].status++;
        break;
    }
}

void GezwelStatus(progdata)
Progdata *progdata;
{
    switch (progdata->living[GEZWEL].status)
    {
    case 0:
        progdata->living[GEZWEL].status = 1;
        break;
    case 3:
        progdata->status.lifepoints -= BIG_WOUND;
        /* Deliberate fall-through! */
    case 2:
        progdata->rooms[75].connect[NORTH] = 70;
        progdata->living[GEZWEL].status = 4;
        break;
    }
}

void BarbecueStatus(progdata)
Progdata *progdata;
{
    switch (progdata->living[BARBECUE].status)
    {
    case 1:
    case 2:
        progdata->living[BARBECUE].status = 3;
        break;
    case 4:
        progdata->items[KOEKJE].room = progdata->status.curroom;
        progdata->living[BARBECUE].status = 0;
        break;
    }
}

void ApplySimmeringForest(progdata)
Progdata *progdata;
{
    char *smeulendbos = progdata->strings[SIMMERING_FOREST];
    int i, j;

    for (i = 0; i < 20; i += 5)
        for (j = 0; j < 2; j++)
            if (i + j != 6)
                progdata->rooms[i + j].descript = smeulendbos;
    progdata->rooms[2].descript = smeulendbos;
    progdata->rooms[4].descript = smeulendbos;
    progdata->rooms[7].descript = smeulendbos;
}

void BoomStatus(progdata)
Progdata *progdata;
{
    switch (progdata->living[BOOM].status)
    {
    case 2:
        progdata->status.lifepoints -= BIG_WOUND;
        /* Deliberate fall-through! */
    case 1:
        ApplySimmeringForest(progdata);

        progdata->items[GROENKRISTAL].room = 4;
        progdata->living[DIAMANT].status = 1;
        progdata->living[BOOM].status = 3;
        break;
    }
}

void DrakeKopStatus(progdata)
Progdata *progdata;
{
    switch (progdata->living[DRAKEKOP].status)
    {
    case 5:
        progdata->rooms[32].connect[NORTH] = 27;
        /* Deliberate fall-through! */
    case 1:
    case 3:
        progdata->living[DRAKEKOP].status++;
        break;
    }
}

bool LavaStatus(progdata)
Progdata *progdata;
{
    switch (progdata->living[LAVA].status)
    {
    case 0:
        /* No text was shown for status 0 as such. Depending on the player wearing 
           the heat suit or not, we now decide what status text to show. */
        PrintLivingStatus(LAVA, progdata->items[HITTEPAK].room == OWNED ? 2 : 3);

        if (progdata->items[HITTEPAK].room != OWNED)
        {
            progdata->status.curroom = 54; /* Back one cave */
            progdata->status.lifepoints -= BIG_WOUND;
            return FALSE;
        }
        break;
    case 1:
        /* Mission completed! */
        ForceExit(progdata);
    }
    return TRUE;
}

void PapierStatus(progdata)
Progdata *progdata;
{
    switch (progdata->living[PAPIER].status)
    {
    case 1:
        progdata->items[PAPIERITEM].room = 66;
        progdata->living[PAPIER].status = 2;
        break;
    }
}
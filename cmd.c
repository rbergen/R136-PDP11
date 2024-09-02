#include "r136.h"
#include "ldstr.h"
#include "items.h"
#include "living.h"

void DoGebruik();
void UseItemToStatus();
void DoCombineer();
void DoLeg();
void DoPak();
void DoBekijk();
void DoAfwachten();
void DoStatus();
void ParseInput();
int FindOwnedItemNum();
int FindLayingItemNum();

/* File-specific flag values */
#define NO_COMMAND          -1
#define AMBIGUOUS_COMMAND   -2
#define AMBIGUOUS_ITEM      -2
#define INFINITE_POINTS     -1

/* File numbers for specific topics. Other 'c' files contain "use" texts
   for corresponding items */
#define COMMAND_LINE_TEXTS      2
#define AFWACHTEN_ACTIONS       4
#define STATUS_LINE_TEXTS       5
#define HELP_TEXT               6
#define PARSE_ERROR_TEXTS       7
#define GAME_OVER_TEXT          8

/* Lamp */
#define LAMP_FADES              0
#define LAMP_DIES               1
#define LAMP_OFF                2
#define GONE_DARK               3
#define LAMP_ON                 4
#define LAMP_NEEDS_BATTERIES    5
#define NEED_LAMP               6
#define NEED_BATTERIES          7

/* Sword */
#define STRIKE_AND_MISS         0
#define STRIKE_AND_HIT          1
#define BEAST_SEVERELY_WOUNDED  2
#define WANT_TO_STRIKE_AGAIN    3
#define SWORD_TEXT_COUNT        4

/* Command line texts */
#define THERES_NO_USE           0
#define NO_WOUNDS_TO_TREAT      1
#define CANT_GO_THAT_WAY        2
#define WANT_TO_LET_EARTH_DIE   3
#define GOOD_DECISION           4
#define COMBINES_TO_NOTHING     5
#define YOU_PUT_DOWN            6
#define POCKETS_FULL            7
#define YOU_PICK_UP             8
#define TOO_DARK_TO_LOOK        9

/* Status line texts */
#define FIRST_LINE              0
#define LAMP_IS_ON              1
#define LAMP_IS_OFF             2
#define YOU_POSSESS_NOTHING     3
#define YOU_POSSESS             4

/* Parse error line texts */
#define NO_COMMAND_GIVEN        0
#define INVALID_COMMAND         1
#define AMBIGUOUS_ABBREVIATION  2
#define COMBINEER_SYNTAX        3
#define PAK_SYNTAX              4
#define SIC_SYNTAX              5
#define CANT_COMBINE_WITH_SELF  6
#define ITEM_NOT_HERE           7
#define YOU_HAVE_NO             8

bool IsConfirmed(progdata)
Progdata *progdata;
{
    char ch;

    ch = agetchar(Str(YESNO));

    if (isupper(ch))
        ch = tolower(ch);

    /* Make sure the character we got remains on screen */
    putch(ch);

    return ch == Str(YESNO)[0];
}

bool DoAction(progdata)
Progdata *progdata;
{
    char inpstr[65];
    Parsedata parsedata;
    int cury;
    static int gnu_rooms[5] = {44, 47, 48, 49, 54};

    if (progdata->status.lifepoints <= 0)
    {
        PrintFile('c', GAME_OVER_TEXT, FALSE);
        ForceExit(progdata);
    }

    if (progdata->status.lamppoints > 0 && progdata->status.lamp)
    {
        switch(--(progdata->status.lamppoints))
        {
        case 10:
            cputs(GetSingleLineText('c', ZAKLAMP, LAMP_FADES, TRUE));
            break;
        case 0:
            cputs(GetSingleLineText('c', ZAKLAMP, LAMP_DIES, TRUE));
            progdata->status.lamp = FALSE;
            break;
        }
    }

    do
    {
        cury = wherey();
        memset(inpstr, ' ', 64);
        inpstr[64] = 0;

        do
        {
            cputs("\r> ");
            strinp(" abcdefghijklmnopqrstuvwxyz", inpstr, wherex(), wherey(), -1, 0, 0);
            /* This is a hacky way to ensure the input the user entered is not deleted
               if the parsing succeeds. */ 
            gotoxy(67, cury);

            ParseInput(progdata, inpstr, &parsedata);
        }
        while (parsedata.error);

        cputs("\n\n");

        switch(parsedata.command)
        {
        case EAST:
        case WEST:
        case NORTH:
        case SOUTH:
        case UP:
        case DOWN:
            if (progdata->rooms[progdata->status.curroom].connect[parsedata.command] == NO_ROOM)
            {
                cputs(GetSingleLineText('c', COMMAND_LINE_TEXTS, CANT_GO_THAT_WAY, TRUE));
                break;
            }

            if (progdata->living[GNOE].room == progdata->status.curroom && progdata->living[GNOE].status != 3)
                progdata->living[GNOE].room = gnu_rooms[rnd(5)];

            progdata->status.curroom = progdata->rooms[progdata->status.curroom].connect[parsedata.command];
            if (progdata->status.paperpos != 6)
            {
                if (progdata->status.curroom == progdata->paperroute[progdata->status.paperpos])
                    progdata->status.paperpos++;
                else
                    progdata->status.paperpos = 0;

                if (progdata->status.paperpos == 6)
                    progdata->living[PAPIER].status = 1;
            }
            return TRUE;

        case GEBRUIK:
            DoGebruik(progdata, &parsedata);
            break;

        case COMBINEER:
            DoCombineer(progdata, &parsedata);
            break;

        case PAK:
            DoPak(progdata, &parsedata);
            break;

        case LEG:
            DoLeg(progdata, &parsedata);
            break;

        case BEKIJK:
            DoBekijk(progdata, &parsedata);
            break;

        case AFWACHTEN:
            DoAfwachten();
            break;

        case EINDE:
            cputs(GetSingleLineText('c', COMMAND_LINE_TEXTS, WANT_TO_LET_EARTH_DIE, TRUE));
            if (IsConfirmed(progdata))
                return FALSE;

            cputs(GetSingleLineText('c', COMMAND_LINE_TEXTS, GOOD_DECISION, TRUE));
            break;

        case STATUS:
            DoStatus(progdata);
            break;

        case HELP:
            PrintFile('c', HELP_TEXT, FALSE);
            break;
        }

        putch('\n');
    }
    while (parsedata.command == STATUS || parsedata.command == HELP || parsedata.command == EINDE);

    return TRUE;
}

void DoGebruik(progdata, parsedata)
Progdata *progdata;
Parsedata *parsedata;
{
    switch (progdata->owneditems[parsedata->object1])
    {
    case ZWAARD:
        UseSword(progdata);
        break;
    
    case ZAKLAMP:
        if (progdata->status.lamp)
        {
            progdata->status.lamp = !progdata->status.lamp;
            cputs(GetSingleLineText('c', ZAKLAMP, LAMP_OFF, TRUE));
            if (IsRoomDark(progdata->status.curroom)) 
                cputs(GetSingleLineText('c', ZAKLAMP, GONE_DARK, TRUE));
            putch('\n');
            break;
        }
        if (progdata->status.lamppoints)
        {
            progdata->status.lamp = !progdata->status.lamp;
            cputs(GetSingleLineText('c', ZAKLAMP, LAMP_ON, TRUE));
        }
        else
            cputs(GetSingleLineText('c', ZAKLAMP, LAMP_NEEDS_BATTERIES, TRUE));
        break;
    
    case VERBAND:
        if (progdata->status.lifepoints == 20)
        {
            cputs(GetSingleLineText('c', COMMAND_LINE_TEXTS, NO_WOUNDS_TO_TREAT, TRUE));
            break;
        }

        PrintFile('c', VERBAND, FALSE);
        progdata->status.lifepoints = 20;
        progdata->items[VERBAND].room = NO_ROOM;
        progdata->owneditems[parsedata->object1] = NO_ITEM;
        getch();
        break;

    case TNT:
        PrintFile('c', TNT, FALSE);
        progdata->status.lifepoints--;
        getch();
        break;

    case HITTEPAK:
        PrintFile('c', HITTEPAK, FALSE);
        break;
    
    case GASMASKER:
        PrintFile('c', GASMASKER, FALSE);
        break;
    
    default:
        if (progdata->items[progdata->owneditems[parsedata->object1]].useableon < 0
             || progdata->living[progdata->items[progdata->owneditems[parsedata->object1]].useableon].room != progdata->status.curroom)
        {
            cputs(GetSingleLineText('c', COMMAND_LINE_TEXTS, THERES_NO_USE, TRUE));
            break;
        }

        switch (progdata->owneditems[parsedata->object1])
        {
        case BOT:
            progdata->living[DEUR].status = 1;
            break;
        
        case DISKETTE:
            UseItemToStatus(progdata, DISKETTE, parsedata->object1, COMPUTER, 2);
            break;
        
        case HASJ:
            UseItemToStatus(progdata, HASJ, parsedata->object1, BARBECUE, progdata->living[BARBECUE].status ? 4 : 1);
            break;
        
        case HONDVLEES:
            UseItemToStatus(progdata, HONDVLEES, parsedata->object1, BARBECUE, progdata->living[BARBECUE].status ? 4 : 2);
            break;
        
        case ROODKRISTAL:
        case GROENKRISTAL:
        case BLAUWKRISTAL:
            UseItemToStatus(progdata, progdata->owneditems[parsedata->object1], parsedata->object1, DRAKEKOP, progdata->living[DRAKEKOP].status + 1);
            break;
        
        case KOEKJE:
            UseItemToStatus(progdata, KOEKJE, parsedata->object1, DRAAK, 3);
            break;
        
        case SLAAPMUTS:
            if (progdata->living[DRAAK].status != 4)
            {
                cputs(GetSingleLineText('c', COMMAND_LINE_TEXTS, THERES_NO_USE, TRUE));
                return;
            }
            UseItemToStatus(progdata, SLAAPMUTS, parsedata->object1, DRAAK, 5);
            break;
        
        case BOM:
            UseItemToStatus(progdata, BOM, parsedata->object1, LAVA, 2);
            break;
        
        case VLAMMENWERPER:
            UseItemToStatus(progdata, VLAMMENWERPER, parsedata->object1, BOOM, progdata->items[HITTEPAK].room == OWNED ? 1 : 2);
            break;
        
        case GIFTIGVLEES:
            UseItemToStatus(progdata, GIFTIGVLEES, parsedata->object1, GNOE, 2);
            break;
        
        case BOEKJE:
            UseItemToStatus(progdata, BOEKJE, parsedata->object1, RODETROL, 4);
            break;
        
        case GASGRANAAT:
            UseItemToStatus(progdata, GASGRANAAT, parsedata->object1, GEZWEL, progdata->items[GASMASKER].room == OWNED ? 2 : 3);
            break;
        }
        
        BeastStatus(progdata);
        getch();
        break;
    }
}

void UseSword(progdata)
Progdata *progdata;
{
    char *texts[SWORD_TEXT_COUNT];
    int beast = HELLEHOND;

    LoadStrings(texts, SWORD_TEXT_COUNT, 'c', ZWAARD, TRUE);

    if (progdata->living[beast].room != progdata->status.curroom || !progdata->living[beast].strike)
    {
        beast = PLANT;
        if (progdata->living[beast].room != progdata->status.curroom || !progdata->living[beast].strike)
        {
            cputs(GetSingleLineText('c', COMMAND_LINE_TEXTS, THERES_NO_USE, TRUE));
            return;
        }
    }
    while (TRUE)
    {
        if (rnd(100) > 70)
            cputs(texts[STRIKE_AND_MISS]);
        else
        {
            cputs(texts[STRIKE_AND_HIT]);
            progdata->living[beast].strike--;
        }

        if (progdata->living[beast].strike == 1)
            cputs(texts[BEAST_SEVERELY_WOUNDED]);
        
        if (!progdata->living[beast].strike || rnd(100) > 30)
            break;
        
        cputs(texts[WANT_TO_STRIKE_AGAIN]);
        if (!IsConfirmed(progdata))
        {
            putch('\n');
            break;
        }
        putch('\n');
    }
    putch('\n');

    if (!progdata->living[beast].strike)
    {
        progdata->living[beast].status = 3;
        BeastStatus(progdata);
    }

    getch();
}

void UseItemToStatus(progdata, item, ownedindex, beast, status)
Progdata *progdata;
int item, ownedindex, beast, status;
{
    progdata->items[item].room = NO_ROOM;
    progdata->owneditems[ownedindex] = NO_ITEM;
    progdata->living[beast].status = status;
}

void DoCombineer(progdata, parsedata)
Progdata *progdata;
Parsedata *parsedata;
{
    if (progdata->items[progdata->owneditems[parsedata->object1]].useableon > -2
         || -(progdata->items[progdata->owneditems[parsedata->object1]].useableon + 2) != progdata->owneditems[parsedata->object2])
    {
        cputs(GetSingleLineText('c', COMMAND_LINE_TEXTS, COMBINES_TO_NOTHING, TRUE));
        return;
    }

    switch(progdata->owneditems[parsedata->object1])
    {
    case ZAKLAMP:
    case BATTERIJEN:
        PrintFile('c', BATTERIJEN, FALSE);
        progdata->status.lamppoints = INFINITE_POINTS;
        progdata->items[BATTERIJEN].room = NO_ROOM;
        if (progdata->owneditems[parsedata->object1] == BATTERIJEN)
            progdata->owneditems[parsedata->object1] = NO_ITEM;
        else
            progdata->owneditems[parsedata->object2] = NO_ITEM;
        break;
    case GASPATROON:
    case ONTSTEKING:
        PrintFile('c', GASPATROON, FALSE);
        progdata->items[GASPATROON].room = NO_ROOM;
        progdata->items[ONTSTEKING].room = NO_ROOM;
        progdata->items[GASGRANAAT].room = OWNED;
        progdata->owneditems[parsedata->object1] = GASGRANAAT;
        progdata->owneditems[parsedata->object2] = NO_ITEM;
        break;
    }
}

void DoLeg(progdata, parsedata)
Progdata *progdata;
Parsedata *parsedata;
{
    if (progdata->owneditems[parsedata->object1] == ZAKLAMP)
    {
        cputs(GetSingleLineText('c', ZAKLAMP, NEED_LAMP));
        return;
    }
    if (progdata->owneditems[parsedata->object1] == BATTERIJEN)
    {
        cputs(GetSingleLineText('c', BATTERIJEN, NEED_BATTERIES));
        return;
    }
    wprintw(mainscr, GetSingleLineText('c', COMMAND_LINE_TEXTS, YOU_PUT_DOWN, TRUE), progdata->items[progdata->owneditems[parsedata->object1]].name);
    progdata->items[progdata->owneditems[parsedata->object1]].room = progdata->status.curroom;
    progdata->owneditems[parsedata->object1] = NO_ITEM;
}

void DoPak(progdata, parsedata)
Progdata *progdata;
Parsedata *parsedata;
{
    int i;

    for (i = 0; i < MAX_OWNED_ITEMS; i++)
        if (progdata->owneditems[i] == NO_ITEM)
            break;

    if (i == MAX_OWNED_ITEMS)
    {
        wprintw(mainscr, GetSingleLineText('c', COMMAND_LINE_TEXTS, POCKETS_FULL, TRUE), progdata->items[parsedata->object1].name);
        return;
    }

    wprintw(mainscr, GetSingleLineText('c', COMMAND_LINE_TEXTS, YOU_PICK_UP, TRUE), progdata->items[parsedata->object1].name);

    progdata->items[parsedata->object1].room = OWNED;
    progdata->owneditems[i] = parsedata->object1;
}

void DoBekijk(progdata, parsedata)
Progdata *progdata;
Parsedata *parsedata;
{
    if (IsRoomDark(progdata->status.curroom) && !progdata->status.lamp)
    {
        cputs(GetSingleLineText('c', COMMAND_LINE_TEXTS, TOO_DARK_TO_LOOK, TRUE));
        return;
    }

    cputs(GetSingleLineText('i', 1, progdata->owneditems[parsedata->object1], TRUE));
    putch('\n');
}

void DoAfwachten()
{
    static bool loaded = FALSE;
    static char*actions[5];

    if (!loaded)
    {
        LoadStrings(actions, 5, 'c', AFWACHTEN_ACTIONS, TRUE);
        loaded = TRUE;
    }

    cputs(actions[rnd(5)]);
}

void DoStatus(progdata)
Progdata *progdata;
{
    int i;
    int count = 0;

    wprintw(mainscr, GetSingleLineText('c', STATUS_LINE_TEXTS, FIRST_LINE, TRUE), progdata->status.lifepoints);
    if (progdata->items[ZAKLAMP].room == OWNED)
        cputs(GetSingleLineText('c', STATUS_LINE_TEXTS, progdata->status.lamp ? LAMP_IS_ON : LAMP_IS_OFF, TRUE));

    for (i = 0; i < 10; i++)
        if (progdata->owneditems[i] != NO_ITEM)
            count++;

    if (!count)
        cputs(GetSingleLineText('c', STATUS_LINE_TEXTS, YOU_POSSESS_NOTHING, TRUE));
    else
    {
        cputs(GetSingleLineText('c', STATUS_LINE_TEXTS, YOU_POSSESS, TRUE));
        for (i = 0; i < 10; i++)
            if (progdata->owneditems[i] != NO_ITEM)
            {
                cputs("    ");
                cputs(progdata->items[progdata->owneditems[i]].name);
                putch('\n');
            }
    }
}

void ShowInputError(format, va_alist)
char *format;
va_dcl
{
    va_list argp;
    char spaces[80];
    /* I hate this fixed buffer size thing... But let's pretend it didn't matter in the 
       PDP-11 days. */
    char buffer[255];

    cputs("\r< ");

    va_start(argp);
    vsprintf(buffer, format, argp);
    va_end(argp);

    cputs(buffer);
    getch();
}

void ProcessParseError(parsedata, message)
Parsedata *parsedata;
char *message;
{
    ShowInputError(message);
    parsedata->error = TRUE;
}

bool FindAndCheckOwnedItem(progdata, parsedata, target, itemname)
Progdata *progdata;
Parsedata *parsedata;
char *target, *itemname;
{
    *target = FindOwnedItemNum(progdata, itemname);
    if (*target < 0)
    {
        parsedata->error = TRUE;
        return FALSE;
    }

    return TRUE;
}

bool ParseSingleItemCommand(progdata, parsedata, eoword, command)
Progdata *progdata;
Parsedata *parsedata;
char* eoword, *command;
{
    if (*eoword != ' ')
    {
        ShowInputError(GetSingleLineText('c', PARSE_ERROR_TEXTS, SIC_SYNTAX, FALSE), command);
        parsedata->error = TRUE;
        return FALSE;
    }

    return FindAndCheckOwnedItem(progdata, parsedata, &(parsedata->object1), eoword + 1);
}

#define PARSE_ERROR_RETURN(message)         { ProcessParseError(parsedata, message); return; }
#define RETURN_IF_NOT_FOUND(item, itemname) { if (!FindAndCheckOwnedItem(progdata, parsedata, item, itemname)) return; }

void ParseInput(progdata, inpstr, parsedata)
Progdata *progdata;
char *inpstr;
Parsedata *parsedata;
{
    char *eoword, *curp;
    char workstr[65], itemname[25];
    int i;

    parsedata->error = FALSE;

    strcpy(workstr, inpstr);

    for (i = strlen(workstr) - 1; i >= 0 && workstr[i] == ' '; i--);
    workstr[i + 1] = 0;
    for (curp = workstr; curp < (workstr + strlen(workstr)) && *curp == ' '; curp++);

    eoword = strchr(curp, ' ');
    if (eoword == NULL)
        eoword = curp + strlen(curp);

    if (eoword == curp)
        PARSE_ERROR_RETURN(GetSingleLineText('c', PARSE_ERROR_TEXTS, NO_COMMAND_GIVEN, FALSE));

    parsedata->command = NO_COMMAND;

    for (i = 0; i < COMMAND_COUNT && parsedata->command != AMBIGUOUS_COMMAND; i++)
        if (!strncmp(progdata->commands[i], curp, (int)(eoword - curp)))
            parsedata->command = parsedata->command == NO_COMMAND ? i : AMBIGUOUS_COMMAND;

    switch (parsedata->command)
    {
    case NO_COMMAND:
        PARSE_ERROR_RETURN(GetSingleLineText('c', PARSE_ERROR_TEXTS, INVALID_COMMAND, FALSE));

    case AMBIGUOUS_COMMAND:
        ShowInputError(GetSingleLineText('c', PARSE_ERROR_TEXTS, AMBIGUOUS_ABBREVIATION, FALSE), eoword + 1);
        parsedata->error = TRUE;
        break;

    case GEBRUIK:
    case LEG:
    case BEKIJK:
        ParseSingleItemCommand(progdata, parsedata, eoword, progdata->commands[parsedata->command]);
        break;

    case COMBINEER:
        if (*eoword != ' ' || (!strstr(curp, AND) && !strstr(curp, WITH)))
            PARSE_ERROR_RETURN(GetSingleLineText('c', PARSE_ERROR_TEXTS, COMBINEER_SYNTAX, FALSE));

        curp = eoword + 1;
        if ((eoword = strstr(curp, AND)) == NULL)
            eoword = strstr(curp, WITH);

        strncpy(itemname, curp, ((eoword - curp) < 25 ? (int)(eoword - curp) : 24));
        itemname[((eoword - curp) < 25 ? (int)(eoword - curp) : 24)] = 0;

        RETURN_IF_NOT_FOUND(parsedata->object1, itemname);

        curp = eoword + strlen(strstr(curp, AND) == curp ? AND : WITH);
        strncpy(itemname, curp, (strlen(curp) < 25 ? strlen(curp) : 24));
        itemname[(strlen(curp) < 25 ? strlen(curp) : 24)] = 0;

        RETURN_IF_NOT_FOUND(parsedata->object2, itemname);

        if (parsedata->object1 == parsedata->object2)
            ProcessParseError(parsedata, GetSingleLineText('c', PARSE_ERROR_TEXTS, CANT_COMBINE_WITH_SELF, FALSE));
        break;

    case PAK:
        if (*eoword != ' ')
            PARSE_ERROR_RETURN(GetSingleLineText('c', PARSE_ERROR_TEXTS, PAK_SYNTAX, FALSE));

        parsedata->object1 = FindLayingItemNum(progdata, eoword + 1);

        if (parsedata->object1 < 0)
        {
            ShowInputError(GetSingleLineText('c', PARSE_ERROR_TEXTS, parsedata->object1 == NO_ITEM
                ? ITEM_NOT_HERE : AMBIGUOUS_ABBREVIATION), eoword + 1);
            parsedata->error = TRUE;
        }
        break;
    }
}

int FindOwnedItemNum(progdata, itemname)
Progdata *progdata;
char *itemname;
{
    int item = NO_ITEM;
    int i;

    for (i = 0; i < MAX_OWNED_ITEMS && item != AMBIGUOUS_ITEM; i++)
        if (progdata->owneditems[i] != NO_ITEM
            && strstr(progdata->items[progdata->owneditems[i]].name, itemname))
        {    
            item = item == NO_ITEM ? i : AMBIGUOUS_ITEM;
        }

   if (item < 0)
        ShowInputError(GetSingleLineText('c', PARSE_ERROR_TEXTS, item == NO_ITEM
            ? YOU_HAVE_NO : AMBIGUOUS_ABBREVIATION, FALSE), itemname);

    return item;
}

int FindLayingItemNum(progdata, itemname)
Progdata *progdata;
char *itemname;
{
    int item = NO_ITEM;
    int i;

    if (IsRoomDark(progdata->status.curroom) && !progdata->status.lamp)
        return NO_ITEM;

    for (i = 0; i < ITEM_COUNT && item != AMBIGUOUS_ITEM; i++)
        if (progdata->status.curroom == progdata->items[i].room
            && strstr(progdata->items[i].name, itemname))
        {
            item = item == NO_ITEM ? i : AMBIGUOUS_ITEM;
        }

    return item;
}



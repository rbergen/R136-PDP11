#include "r136.h"
#include "ldstr.h"
#include "items.h"
#include "living.h"
#include "cmd.h"

bool DoGebruik();
void UseItemToStatus();
void DoCombineer();
void DoLeg();
void DoPak();
void DoBekijk();
void DoAfwachten();
void DoStatus();
void DoHelp();
void ParseInput();
int FindOwnedItemNum();
int FindLayingItemNum();

#define LAMP_FADES              0
#define LAMP_DIES               1
#define LAMP_OFF                2
#define GONE_DARK               3
#define LAMP_ON                 4
#define LAMP_NEEDS_BATTERIES    5
#define NEED_LAMP               6
#define NEED_BATTERIES          7

#define STRIKE_AND_MISS         0
#define STRIKE_AND_HIT          1
#define BEAST_SEVERELY_WOUNDED  2
#define WANT_TO_STRIKE_AGAIN    3
#define SWORD_TEXT_COUNT        4

#define COMMAND_LINE_TEXTS      2

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


void DoAction(progdata)
Progdata *progdata;
{
    char inpstr[65];
    Parsedata parsedata;
    int cury;
    int gnu_rooms[5] = {44, 47, 48, 49, 54};

    if (progdata->status.lifepoints <= 0)
    {
        cputs("Badend in je eigen bloed bezwijk je aan je verwondingen. Terwijl je liggend op\n"
              "de grond met moeite naar adem snakt, hoor je in de verte een luid gelach.\n"
              "Dan zakken je ogen dicht en stopt je hart met kloppen.\n"
              "Op hetzelfde moment ontploft de bom en sterft de aarde met jou.\n\n");
        ForceExit();
    }

    if (progdata->status.lamppoints > 0 && progdata->status.lamp)
    {
        switch(--progdata->status.lamppoints)
        {
        case 10:
            cputs(GetSingleLineText('c', ZAKLAMP, LAMP_FADES));
            break;
        case 0:
            cputs(GetSingleLineText('c', ZAKLAMP, LAMP_DIES));
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
            gotoxy(0, cury);
            cputs("> ");
            strinp(" abcdefghijklmnopqrstuvwxyz", inpstr, wherex(), wherey(), -1, 0, 0);
            gotoxy(0, cury);
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
            break;

        case GEBRUIK:
            DoGebruik(progdata, &parsedata);

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
            if (IsConfirmed())
                return FALSE;

            cputs(GetSingleLineText('c', COMMAND_LINE_TEXTS, GOOD_DECISION, TRUE));
            break;

        case STATUS:
            DoStatus(progdata);
            break;
        case HELP:
            DoHelp();
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
            cputs(GetSingleLineText('c', ZAKLAMP, LAMP_OFF));
            if (IsRoomDark(progdata->status.curroom)) 
                cputs(GetSingleLineText('c', ZAKLAMP, GONE_DARK));
            putch('\n');
            break;
        }
        if (progdata->status.lamppoints)
        {
            progdata->status.lamp = !progdata->status.lamp;
            cputs(GetSingleLineText('c', ZAKLAMP, LAMP_ON));
        }
        else
            cputs(GetSingleLineText('c', ZAKLAMP, LAMP_NEEDS_BATTERIES));
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
                return TRUE;
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
        if (!IsConfirmed())
        {
            putch('\n');
            break;
        }
        putch('\n');
    }
    putch('\n');`

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

    cputs(progdata->items[progdata->owneditems[parsedata.object1]].descript);
    putch('\n');
}

void DoAfwachten(void)
{
    static const char *actions[] =
    {
        "Je pulkt wat in je neus.\n",
        "Je krabt wat achter je oren.\n",
        "Je gaapt even uitgebreid.\n",
        "Je trekt je broek even op.\n",
        "Je pulkt wat smeer uit je oren.\n"
    };

    cputs(actions[rnd(5)]);
}

void DoStatus(Progdata &progdata)
{
    int i;
    int count = 0;

    cputs("--- STATUSRAPPORT ---\n\n");
    wprintw(mainscr, "Je hebt nog %d levenspunten.\n", progdata->status.lifepoints);
    if (progdata->items[ZAKLAMP].room == OWNED)
        wprintw(mainscr, "Je zaklamp staat %s.\n", progdata->status.lamp ? "aan" : "uit");

    for (i = 0; i < 10; i++)
        if (progdata->owneditems[i] != NO_ITEM)
            count++;

    if (!count)
        cputs("Je hebt niets.\n");
    else
    {
        cputs("Je hebt in je bezit:\n");
        for (i = 0; i < 10; i++)
            if (progdata->owneditems[i] != NO_ITEM)
            {
                cputs("    ");
                wprintw(mainscr, progdata->items[progdata->owneditems[i]].name);
                putch('\n');
            }
    }
}

#define ITEM 	  					 "<voorwerp>"
#define AND_WITH 					 " en/met "
#define WITH 						 " met "
#define AND  						 " en "
#define INVALID_COMMAND 		 "ongeldig commando gegeven"
#define SYNTAX_PREFIX  			 "syntax: "
#define AMBIGUOUS_ABBREVIATION "de afkorting \"%s\" is dubbelzinnig"

#define SIMPLE_COMMAND(command) "   " command "\n"
#define ITEM_COMMAND(command)   SIMPLE_COMMAND(command " " ITEM)

void DoHelp(void)
{
   textcolor(WHITE);
    cputs("--- HELP ---\n\n");
   textcolor(LIGHTGRAY);
    cputs("Commando's:\n"
         SIMPLE_COMMAND(CMD_EAST)
            SIMPLE_COMMAND(CMD_WEST)
            SIMPLE_COMMAND(CMD_NORTH)
            SIMPLE_COMMAND(CMD_SOUTH)
            SIMPLE_COMMAND(CMD_UP)
            SIMPLE_COMMAND(CMD_DOWN)
            ITEM_COMMAND(CMD_GEBRUIK)
            SIMPLE_COMMAND(CMD_COMBINEER " " ITEM AND_WITH ITEM)
            ITEM_COMMAND(CMD_PAK)
            ITEM_COMMAND(CMD_LEG)
            ITEM_COMMAND(CMD_BEKIJK)
            SIMPLE_COMMAND(CMD_AFWACHTEN)
            SIMPLE_COMMAND(CMD_EINDE)
            SIMPLE_COMMAND(CMD_STATUS)
            SIMPLE_COMMAND(CMD_HELP)
         );
}

void ShowInputError(const char *format, ...)
{
    char spaces[80];
    memset(spaces, ' ', 79);
    spaces[79] = 0;
       cputs(spaces);
       textcolor(LIGHTRED);
       cputs("\r< ");

     /* I hate this fixed buffer size thing... But let's pretend it didn't matter in the 
       PDP-11 days. */
   char buffer[255];
   va_list args;

   va_start(args, format);
   vsprintf(buffer, format, args);
   va_end(args);

    textcolor(LIGHTGRAY);
   cputs(buffer);
   getch();
}

inline void ProcessParseError(Parsedata &parsedata, const char *message)
{
    ShowInputError(message);
    parsedata.error = TRUE;
}

inline bool FindAndCheckOwnedItem(Progdata &progdata, Parsedata &parsedata, char &target, const char *itemname)
{
   target = FindOwnedItemNum(progdata, itemname);
    if (target < 0)
   {
        parsedata.error = TRUE;
      return FALSE;
   }

    return TRUE;
}

inline bool ParseSingleItemCommand(Progdata &progdata, Parsedata &parsedata, const char* eoword, const char *command)
{
    if (*eoword != ' ')
   {
       ShowInputError(SYNTAX_PREFIX "%s " ITEM, command);
        parsedata.error = TRUE;
       return FALSE;
   }

    return FindAndCheckOwnedItem(progdata, parsedata, parsedata.object1, eoword + 1);
}

#define PARSE_ERROR_RETURN(message) 		  { ProcessParseError(parsedata, message); return; }
#define PARSE_SYNTAX_ERROR_RETURN(message)  PARSE_ERROR_RETURN(SYNTAX_PREFIX message)
#define RETURN_IF_NOT_FOUND(item, itemname) { if (!FindAndCheckOwnedItem(progdata, parsedata, item, itemname)) return; }
#define RETURN_IF_SIC_INVALID(command) 	  { if (!ParseSingleItemCommand(progdata, parsedata, eoword, command)) return; }

void ParseInput(Progdata &progdata, const char *inpstr, Parsedata &parsedata)
{
    char *eoword;
    char *curp;
    char workstr[65];
    char itemname[25];
    int i;

    parsedata.error = FALSE;

    strcpy(workstr, inpstr);

    for (i = strlen(workstr) - 1; i >= 0 && workstr[i] == ' '; i--);
    workstr[i + 1] = 0;
    for (curp = workstr; curp < (workstr + strlen(workstr)) && *curp == ' '; curp++);

    eoword = strchr(curp, ' ');
    if (eoword == NULL)
        eoword = curp + strlen(curp);

    if (eoword == curp)
       PARSE_ERROR_RETURN("geen commando gegeven")

    parsedata.command = NO_COMMAND;

    for (i = 0; i < 15; i++)
        if (!strncmp(cmds[i], curp, int(eoword - curp)))
        {
          parsedata.command = i;
         break;
      }

    switch (parsedata.command)
    {
    case NO_COMMAND:
        ProcessParseError(parsedata, INVALID_COMMAND);
        return;
    case GEBRUIK:
      RETURN_IF_SIC_INVALID(CMD_GEBRUIK);
        break;
    case LEG:
      RETURN_IF_SIC_INVALID(CMD_LEG);
        break;
    case BEKIJK:
      RETURN_IF_SIC_INVALID(CMD_BEKIJK);
        break;
    case COMBINEER:
        if (*eoword != ' ' || (!strstr(curp, AND) && !strstr(curp, WITH)))
          PARSE_SYNTAX_ERROR_RETURN(CMD_COMBINEER " " ITEM AND_WITH ITEM);

        curp = eoword + 1;
        if ((eoword = strstr(curp, AND)) == NULL)
            eoword = strstr(curp, WITH);

        strncpy(itemname, curp, ((eoword - curp) < 25 ? int(eoword - curp) : 24));
        itemname[((eoword - curp) < 25 ? int(eoword - curp) : 24)] = 0;

      RETURN_IF_NOT_FOUND(parsedata.object1, itemname);

        curp = eoword + strlen(strstr(curp, AND) == curp ? AND : WITH);
        strncpy(itemname, curp, (strlen(curp) < 25 ? strlen(curp) : 24));
        itemname[(strlen(curp) < 25 ? strlen(curp) : 24)] = 0;

      RETURN_IF_NOT_FOUND(parsedata.object2, itemname);

        if (parsedata.object1 == parsedata.object2)
            PARSE_ERROR_RETURN("je kunt een voorwerp niet met zichzelf combineren");
        break;
    case PAK:
        if (*eoword != ' ')
          PARSE_SYNTAX_ERROR_RETURN(CMD_PAK " " ITEM);

        parsedata.object1 = FindLayingItemNum(progdata, eoword + 1);

      if (parsedata.object1 < 0)
      {
          ShowInputError(parsedata.object1 == NO_ITEM
              ? "je ziet hier geen \"%s\" die je kunt meenemen"
              : AMBIGUOUS_ABBREVIATION, eoword + 1);
         parsedata.error = TRUE;
         return;
      }
        break;
    }
}

int FindOwnedItemNum(Progdata &progdata, const char *itemname)
{
    int item = NO_ITEM;
    int i;

    for (i = 0; i < 10 && item != AMBIGUOUS_ITEM; i++)
        if (progdata->owneditems[i] != NO_ITEM)
            if (strstr(progdata->items[progdata->owneditems[i]].name, itemname))
                item = item == NO_ITEM ? i : AMBIGUOUS_ITEM;

   if (item < 0)
      ShowInputError(item == NO_ITEM
          ? "je hebt geen \"%s\""
          : AMBIGUOUS_ABBREVIATION, itemname);

    return item;
}

int FindLayingItemNum(Progdata &progdata, const char *itemname)
{
    int item = NO_ITEM;
    int i;

    if (progdata->status.curroom != 61
       && progdata->status.curroom != 31
      && progdata->status.curroom >= 20
      && !progdata->status.lamp)
    {
       return NO_ITEM;
   }

    for (i = 0; i < 25 && item != AMBIGUOUS_ITEM; i++)
        if (progdata->status.curroom == progdata->items[i].room
          && strstr(progdata->items[i].name, itemname))
      {
            item = (item == NO_ITEM ? i : AMBIGUOUS_ITEM);
      }

    return item;
}



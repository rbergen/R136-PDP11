#include "r136.h"

#define CMD_EAST		 "oost"
#define CMD_WEST		 "west"
#define CMD_NORTH		 "noord"
#define CMD_SOUTH		 "zuid"
#define CMD_UP			 "klim"
#define CMD_DOWN		 "daal"
#define CMD_GEBRUIK	 "gebruik"
#define CMD_COMBINEER "combineer"
#define CMD_PAK		 "pak"
#define CMD_LEG		 "leg"
#define CMD_BEKIJK	 "bekijk"
#define CMD_AFWACHTEN "afwachten"
#define CMD_EINDE		 "einde"
#define CMD_STATUS	 "status"
#define CMD_HELP		 "help"

const char *cmds[15] = {
	CMD_EAST,
   CMD_WEST,
   CMD_NORTH,
   CMD_SOUTH,
   CMD_UP,
   CMD_DOWN,
   CMD_GEBRUIK,
	CMD_COMBINEER,
   CMD_PAK,
   CMD_LEG,
   CMD_BEKIJK,
   CMD_AFWACHTEN,
   CMD_EINDE,
   CMD_STATUS,
   CMD_HELP
};

#define THERES_NO_USE  			 "Dat heeft geen zin.\n\n"

bool DoGebruik(Progdata &progdata, Parsedata &parsedata);
void UseItemToStatus(Progdata &progdata, int item, int ownedindex, int beast, int status);
void DoCombineer(Progdata &progdata, Parsedata &parsedata);
void DoLeg(Progdata &progdata, Parsedata &parsedata);
void DoPak(Progdata &progdata, Parsedata &parsedata);
void DoBekijk(Progdata &progdata, Parsedata &parsedata);
void DoAfwachten(void);
void DoStatus(Progdata &progdata);
void DoHelp(void);
void ParseInput(Progdata &progdata, const char *inpstr, Parsedata &parsedata);
int FindOwnedItemNum(Progdata &progdata, const char *itemname);
int FindLayingItemNum(Progdata &progdata, const char *itemname);

bool DoAction(Progdata &progdata)
{
	char inpstr[65];
	Parsedata parsedata;
	int cury;

	if (progdata.status.lifepoints <= 0)
	{
		cputs("Badend in je eigen bloed bezwijk je aan je verwondingen. Terwijl je liggend op\n"
      		"de grond met moeite naar adem snakt, hoor je in de verte een luid gelach.\n"
				"Dan zakken je ogen dicht en stopt je hart met kloppen.\n"
				"Op hetzelfde moment ontploft de bom en sterft de aarde met jou.\n\n");
		ForceExit();
	}

	if (progdata.status.lamppoints > 0 && progdata.status.lamp)
	{
		switch(--progdata.status.lamppoints)
		{
		case 10:
			cputs("De zaklamp gaat zwakker branden.\n\n");
			break;
		case 0:
			cputs("De batterijen van de zaklamp zijn uitgeput. Langzaam dooft hij.\n\n");
			progdata.status.lamp = false;
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
			gotoxy(1, cury);
         textcolor(YELLOW);
			sputs("> ");
			strinp(" abcdefghijklmnopqrstuvwxyz", inpstr, wherex(), wherey(), -1, 0, 0);
			gotoxy(1, cury);
			ParseInput(progdata, inpstr, parsedata);
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
			if (progdata.rooms[progdata.status.curroom].connect[parsedata.command] != NO_ROOM)
			{
				if (progdata.living[GNOE].room == progdata.status.curroom && progdata.living[GNOE].status != 3)
					switch(rnd(5))
					{
					case 0:
						progdata.living[GNOE].room = 44;
						break;
					case 1:
						progdata.living[GNOE].room = 47;
						break;
					case 2:
						progdata.living[GNOE].room = 48;
						break;
					case 3:
						progdata.living[GNOE].room = 49;
						break;
					case 4:
						progdata.living[GNOE].room = 54;
						break;
					}
				progdata.status.curroom = progdata.rooms[progdata.status.curroom].connect[parsedata.command];
				if (progdata.status.paperpos != 6)
				{
					if (progdata.status.curroom == progdata.paperroute[progdata.status.paperpos])
						progdata.status.paperpos++;
					else
						progdata.status.paperpos = 0;
					if (progdata.status.paperpos == 6)
						progdata.living[PAPIER].status = 1;
				}
				return true;
			}
			else
				cputs("Daar kun je niet heen.\n");
			break;
		case GEBRUIK:
			return DoGebruik(progdata, parsedata);
		case COMBINEER:
			DoCombineer(progdata, parsedata);
			break;
		case PAK:
			DoPak(progdata, parsedata);
			break;
		case LEG:
			DoLeg(progdata, parsedata);
			break;
		case BEKIJK:
			DoBekijk(progdata, parsedata);
			break;
		case AFWACHTEN:
			DoAfwachten();
			break;
		case EINDE:
      	textcolor(YELLOW);
			cputs("Weet je zeker dat je de aarde laat vergaan? ");

			if (tolower(agetchar("jJnN")) == 'j')
				return false;
			cputs("\nMooi zo!\n");
			break;
		case STATUS:
			DoStatus(progdata);
			break;
		case HELP:
			DoHelp();
			break;
		}

		cputs("\n");
	}
	while (parsedata.command == STATUS || parsedata.command == HELP || parsedata.command == EINDE);

	return true;
}

bool DoGebruik(Progdata &progdata, Parsedata &parsedata)
{
	int i;

	switch (progdata.owneditems[parsedata.object1])
	{
	case ZWAARD:
		i = HELLEHOND;
		if (progdata.living[i].room != progdata.status.curroom || !progdata.living[i].strike)
		{
			i = PLANT;
			if (progdata.living[i].room != progdata.status.curroom || !progdata.living[i].strike)
			{
				cputs(THERES_NO_USE);
				return true;
			}
		}
		while (true)
		{
			cputs("Je haalt uit met je zwaard");
			if (rnd(100) > 70)
				cputs(", maar het monster ontwijkt.\n");
			else
			{
				cputs(" en je raakt het monster hard.\n");
				progdata.living[i].strike--;
			}
			if (progdata.living[i].strike == 1)
				cputs("\nHet monster is zwaar gewond en je baadt in zijn bloed.\n");
			if (!progdata.living[i].strike || rnd(100) > 30)
				break;
			cputs("\nJe kunt nog een slag uitdelen. ");
         textcolor(YELLOW);
         cputs("Wil je dat? ");
			if (tolower(agetchar("jJnN")) != 'j')
			{
				cputs("\n");
				break;
			}
			cputs("\n");
		}
		cputs("\n");
		if (!progdata.living[i].strike)
		{
			progdata.living[i].status = 3;
			BeastStatus(progdata);
		}
		getch();
		break;
	case ZAKLAMP:
		if (progdata.status.lamp)
		{
			progdata.status.lamp = !progdata.status.lamp;
			wprintw(mainscr, "Je zet de zaklamp uit.%s", ((progdata.status.curroom != 61 && progdata.status.curroom != 31 && progdata.status.curroom >= 20) ? " Je ziet niets meer.\n" : "\n"));
			break;
		}
		if (progdata.status.lamppoints)
		{
			progdata.status.lamp = !progdata.status.lamp;
			cputs("Je zet de zaklamp aan. De straal verlicht de omtrek.\n");
		}
		else
			cputs("Zonder nieuwe batterijen doet-ie het niet...\n");
		break;
	case VERBAND:
		if (progdata.status.lifepoints == 20)
		{
			cputs("Je bent nog helemaal heel!\n");
			break;
		}
		cputs("Je pakt het verband en de pleisters en plaatst ze over je wonden. Je bijt even\n"
      		"op je lippen van de pijn als het verband je nog bloedende wonden raakt.\n\n"
				"Je bent weer zo goed als nieuw.\n");
		progdata.status.lifepoints = 20;
		progdata.items[VERBAND].room = NO_ROOM;
		progdata.owneditems[parsedata.object1] = NO_ITEM;
		getch();
		break;
	case TNT:
		cputs("Je pakt een staafje en gooit het op de grond. Er volgt een explosie die samen-\n"
			   "gaat met een harde knal. Je wordt even verblind door de flits van de ontplof-\n"
			   "fing. Door de klap val je even flauw.\n");
		progdata.status.lifepoints--;
		getch();
		break;
	case HITTEPAK:
		cputs("Je hebt het pak al aan.\n");
		break;
	case GASMASKER:
		cputs("Je hebt het gasmasker al op.\n");
		break;
	default:
		if (progdata.items[progdata.owneditems[parsedata.object1]].useableon < 0
			 || progdata.living[progdata.items[progdata.owneditems[parsedata.object1]].useableon].room != progdata.status.curroom)
		{
			cputs(THERES_NO_USE);
			return true;
		}
		switch (progdata.owneditems[parsedata.object1])
		{
		case BOT:
			progdata.living[DEUR].status = 1;
			break;
		case DISKETTE:
			UseItemToStatus(progdata, DISKETTE, parsedata.object1, COMPUTER, 2);
			break;
		case HASJ:
			UseItemToStatus(progdata, HASJ, parsedata.object1, BARBECUE, progdata.living[BARBECUE].status ? 4 : 1);
			break;
		case HONDVLEES:
			UseItemToStatus(progdata, HONDVLEES, parsedata.object1, BARBECUE, progdata.living[BARBECUE].status ? 4 : 2);
			break;
		case ROODKRISTAL:
		case GROENKRISTAL:
		case BLAUWKRISTAL:
			UseItemToStatus(progdata, progdata.owneditems[parsedata.object1], parsedata.object1, DRAKEKOP, progdata.living[DRAKEKOP].status + 1);
			break;
		case KOEKJE:
			UseItemToStatus(progdata, KOEKJE, parsedata.object1, DRAAK, 3);
			break;
		case SLAAPMUTS:
			if (progdata.living[DRAAK].status != 4)
			{
				cputs(THERES_NO_USE);
				return true;
			}
			UseItemToStatus(progdata, SLAAPMUTS, parsedata.object1, DRAAK, 5);
			break;
		case BOM:
			UseItemToStatus(progdata, BOM, parsedata.object1, LAVA, 2);
			break;
		case VLAMMENWERPER:
			UseItemToStatus(progdata, VLAMMENWERPER, parsedata.object1, BOOM, 1);
			break;
		case GIFTIGVLEES:
			UseItemToStatus(progdata, GIFTIGVLEES, parsedata.object1, GNOE, 2);
			break;
		case BOEKJE:
			UseItemToStatus(progdata, BOEKJE, parsedata.object1, RODETROL, 4);
			break;
		case GASGRANAAT:
			UseItemToStatus(progdata, GASGRANAAT, parsedata.object1, GEZWEL, progdata.items[GASMASKER].room == OWNED ? 2 : 3);
			break;
		}
		BeastStatus(progdata);
		getch();
		break;
	}
	return true;
}

inline void UseItemToStatus(Progdata &progdata, int item, int ownedindex, int beast, int status)
{
	progdata.items[item].room = NO_ROOM;
	progdata.owneditems[ownedindex] = NO_ITEM;
	progdata.living[beast].status = status;
}

void DoCombineer(Progdata &progdata, Parsedata &parsedata)
{
	if (progdata.items[progdata.owneditems[parsedata.object1]].useableon > -2
		 || -(progdata.items[progdata.owneditems[parsedata.object1]].useableon + 2) != progdata.owneditems[parsedata.object2])
	{
		cputs("Dat levert niets bruikbaars op.\n");
		return;
	}

	switch(progdata.owneditems[parsedata.object1])
	{
	case ZAKLAMP:
	case BATTERIJEN:
		cputs("Je schroeft de zaklamp open en schudt totdat de oude batterijen er uit komen\n"
      		"vallen. Daarna steek je de \"trommelbatterijen\" erin en schroeft de lamp weer\n"
				"dicht. Nadat je een paar keer op de zaklantaarn hebt geslagen zie je dat hij\n"
				"het doet.\n");
		progdata.status.lamppoints = INFINITE_POINTS;
		progdata.items[BATTERIJEN].room = NO_ROOM;
		if (progdata.owneditems[parsedata.object1] == BATTERIJEN)
			progdata.owneditems[parsedata.object1] = NO_ITEM;
		else
			progdata.owneditems[parsedata.object2] = NO_ITEM;
		break;
	case GASPATROON:
	case ONTSTEKING:
		cputs("Je plaatst de ontsteker op het mosterdgaspatroon. Na enig friemelen is het\n"
				"resultaat een werkende mosterdgasgranaat.\n");
		progdata.items[GASPATROON].room = NO_ROOM;
		progdata.items[ONTSTEKING].room = NO_ROOM;
		progdata.items[GASGRANAAT].room = OWNED;
		progdata.owneditems[parsedata.object1] = GASGRANAAT;
		progdata.owneditems[parsedata.object2] = NO_ITEM;
		break;
	}
}

void DoLeg(Progdata &progdata, Parsedata &parsedata)
{
	if (progdata.owneditems[parsedata.object1] == ZAKLAMP)
	{
		cputs("Je bent inmiddels zo aan je zaklamp gehecht geraakt dat je hem niet meer kunt\n"
				"missen.\n");
		return;
	}
	if (progdata.owneditems[parsedata.object1] == BATTERIJEN)
	{
		cputs("Je bent inmiddels zo aan je batterijen gehecht geraakt dat je ze niet meer kunt\n"
				"missen.\n");
		return;
	}
	wprintw(mainscr, "Je legt %s neer.\n", progdata.items[progdata.owneditems[parsedata.object1]].name);
	progdata.items[progdata.owneditems[parsedata.object1]].room = progdata.status.curroom;
	progdata.owneditems[parsedata.object1] = NO_ITEM;
}

void DoPak(Progdata &progdata, Parsedata &parsedata)
{
	int i;

	for (i = 0; i < 10; i++)
		if (progdata.owneditems[i] == NO_ITEM)
			break;

	if (i == 10)
	{
		wprintw(mainscr, "Je zakken zitten tjokvol, en je krijgt %s er niet in.\n", progdata.items[parsedata.object1].name);
		return;
	}

	wprintw(mainscr, "Je pakt %s op en steekt deze in " E_GRAVE E_GRAVE "n van je zakken.\n", progdata.items[parsedata.object1].name);

	progdata.items[parsedata.object1].room = OWNED;
	progdata.owneditems[i] = parsedata.object1;
}

void DoBekijk(Progdata &progdata, Parsedata &parsedata)
{
	if (progdata.status.curroom != 61 && progdata.status.curroom != 31 && progdata.status.curroom >= 20 && !progdata.status.lamp)
	{
		cputs("Het is veel te donker om wat dan ook te bekijken.\n");
		return;
	}

	cputs(progdata.items[progdata.owneditems[parsedata.object1]].descript);
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
	wprintw(mainscr, "Je hebt nog %d levenspunten.\n", progdata.status.lifepoints);
	if (progdata.items[ZAKLAMP].room == OWNED)
		wprintw(mainscr, "Je zaklamp staat %s.\n", progdata.status.lamp ? "aan" : "uit");

	for (i = 0; i < 10; i++)
		if (progdata.owneditems[i] != NO_ITEM)
			count++;

	if (!count)
		cputs("Je hebt niets.\n");
	else
	{
		cputs("Je hebt in je bezit:\n");
		for (i = 0; i < 10; i++)
			if (progdata.owneditems[i] != NO_ITEM)
			{
				cputs("    ");
				wprintw(mainscr, progdata.items[progdata.owneditems[i]].name);
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

 	// I hate this fixed buffer size thing... But let's pretend it didn't matter in the MS-DOS days.
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
	parsedata.error = true;
}

inline bool FindAndCheckOwnedItem(Progdata &progdata, Parsedata &parsedata, char &target, const char *itemname)
{
   target = FindOwnedItemNum(progdata, itemname);
	if (target < 0)
   {
		parsedata.error = true;
      return false;
   }

	return true;
}

inline bool ParseSingleItemCommand(Progdata &progdata, Parsedata &parsedata, const char* eoword, const char *command)
{
	if (*eoword != ' ')
   {
   	ShowInputError(SYNTAX_PREFIX "%s " ITEM, command);
    	parsedata.error = true;
   	return false;
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

	parsedata.error = false;

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
         parsedata.error = true;
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
		if (progdata.owneditems[i] != NO_ITEM)
			if (strstr(progdata.items[progdata.owneditems[i]].name, itemname))
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

	if (progdata.status.curroom != 61
   	&& progdata.status.curroom != 31
      && progdata.status.curroom >= 20
      && !progdata.status.lamp)
	{
   	return NO_ITEM;
   }

	for (i = 0; i < 25 && item != AMBIGUOUS_ITEM; i++)
		if (progdata.status.curroom == progdata.items[i].room
      	&& strstr(progdata.items[i].name, itemname))
      {
			item = (item == NO_ITEM ? i : AMBIGUOUS_ITEM);
      }

	return item;
}



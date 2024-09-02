#include <sgtty.h>
#include <sys/time.h>
#include "r136.h"
#include "cmd.h"
#include "items.h"
#include "living.h"
#include "ldstr.h"

char levelcon[11][3] = {
    {6, DOWN, 34},
    {19, DOWN, 21},
    {21, UP, 19},
    {28, DOWN, 58},
    {36, DOWN, 46},
    {38, DOWN, 56},
    {46, UP, 36},
    {56, UP, 38},
    {56, DOWN, 68},
    {58, UP, 28},
    {68, UP, 56}};

char blocked[83][2] = {
    {3, WEST}, {3, EAST}, {3, SOUTH}, {8, WEST},
    {8, EAST}, {8, NORTH}, {8, SOUTH}, {5, EAST},
    {6, WEST}, {11, EAST}, {12, WEST}, {18, WEST},
    {18, EAST}, {18, NORTH}, {19, WEST}, {20, EAST},
    {21, WEST}, {22, EAST}, {22, SOUTH}, {27, NORTH},
    {23, WEST}, {23, EAST}, {24, WEST}, {25, EAST},
    {25, NORTH}, {26, WEST}, {26, EAST}, {27, EAST},
    {27, WEST}, {28, WEST}, {32, NORTH}, {33, EAST},
    {34, WEST}, {31, SOUTH}, {36, NORTH}, {34, SOUTH},
    {37, WEST}, {37, NORTH}, {37, EAST}, {39, NORTH},
    {40, EAST}, {41, WEST}, {41, EAST}, {42, WEST},
    {42, EAST}, {43, WEST},  {43, EAST}, {44, WEST},
    {45, NORTH}, {46, EAST}, {47, WEST}, {52, EAST},
    {48, SOUTH}, {53, NORTH}, {53, EAST}, {54, WEST},
    {55, EAST}, {56, WEST}, {52, SOUTH}, {57, NORTH},
    {53, SOUTH}, {58, EAST}, {59, WEST}, {60, EAST},
    {61, WEST}, {61, SOUTH}, {66, NORTH}, {65, SOUTH},
    {70, NORTH}, {70, EAST}, {71, WEST}, {66, SOUTH},
    {71, NORTH}, {71, EAST}, {72, WEST}, {67, SOUTH},
    {72, NORTH}, {72, EAST}, {73, WEST}, {79, WEST},
    {74, SOUTH}, {75, NORTH}, {79, NORTH}};

beginroom[LIVING_COUNT] = {
    2, 23, 25, 44, 50, 75, 45, 58, 17, 10, 4, 41, 32, 59, 71, 66, 3, 8, 18, 37, 79};
killhits[LIVING_COUNT] = {
    4, 0, 6, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

char itemroom[ITEM_COUNT] = {
    NO_ROOM, 4, NO_ROOM, 5, 13, 20, 22, NO_ROOM, 24, 27, 
    31, 39, 40, 43, 51, NO_ROOM, 52, 60, 61, 62, NO_ROOM, 
    70, 72, NO_ROOM, NO_ROOM};

char workon[ITEM_COUNT] = {
    BARBECUE, NOTHING, DRAKEKOP, NOTHING, DEUR, COMPUTER, BARBECUE,
    DRAKEKOP, DRAAK, LAVA, ON_ITEM(BATTERIJEN), NOTHING, BOOM,
    NOTHING, NOTHING, ON_ITEM(ONTSTEKING), GNOE, ON_ITEM(GASPATROON),
    ON_ITEM(ZAKLAMP), NOTHING, NOTHING, RODETROL, DRAKEKOP, DRAAK, GEZWEL};

bool SetRooms();
bool SetLivings();
bool SetItems();

bool Initialize(progdata)
Progdata *progdata;
{
    int i;
    struct timeval tv;
    
    /* Initialize random number generator */
    gettimeofday(&tv, NULL);
    srandom((int)tv.tv_sec);

    /* Initialize curses */
    initscr();
    erase();    /* Not clrscr(), because we mean the whole window! */

    /* LoadStrings heap-allocates the strings it loads! */
    LoadStrings(progdata->strings, PRELOADED_STRING_COUNT, 'p', 0, TRUE);
    LoadStrings(progdata->commands, COMMAND_COUNT, 'c', 0, FALSE);

    /* Print the title header and keep it from scrolling away */
    PrintHeader(progdata->strings[TITLE]);
    refresh();
    mainscr = subwin(stdscr, 0, 0, 2, 0);
    scrollok(mainscr, TRUE);

    /* Set up our own data structures */
    progdata->paperroute[0] = 69;
    progdata->paperroute[1] = 64;
    progdata->paperroute[2] = 63;
    progdata->paperroute[3] = 62;
    progdata->paperroute[4] = 67;
    progdata->paperroute[5] = 66;

    progdata->status.curroom = 0;
    progdata->status.paperpos = 0;
    progdata->status.lifepoints = 20;
    progdata->status.lamp = FALSE;
    progdata->status.lamppoints = 60;

    for (i = 0; i < 10; i++)
        progdata->owneditems[i] = NO_ITEM;

    return (SetRooms(progdata->rooms)
            && SetLivings(progdata->living)
            && SetItems(progdata->items));
}

/* Doomed are they who don't exit the program after calling this! */
void Deinitialize(progdata)
Progdata *progdata;
{
    int i;

    /* Let's be a responsible citizen and free heap memory for strings */
    for (i = 0; i < PRELOADED_STRING_COUNT; i++)
        free(progdata->strings[i]);

    for (i = 0; i < COMMAND_COUNT; i++)
        free(progdata->commands[i]);

    for (i = 0; i < ITEM_COUNT; i++)
        free(progdata->items[i].name);

    /* Tear down curses */
    delwin(mainscr);
    erase();	/* Not clrscr(), because we mean the whole window! */
    refresh();
    endwin();
}

bool SetRooms(rooms)
Room *rooms;
{
    int i;
    int j;

    for (i = 0; i < 80; i++)
    {
        rooms[i].descript = NULL;
        rooms[i].connect[EAST] = i + 1;
        rooms[i].connect[WEST] = i - 1;
        rooms[i].connect[NORTH] = i - 5;
        rooms[i].connect[SOUTH] = i + 5;
        rooms[i].connect[UP] = rooms[i].connect[DOWN] = NO_ROOM;
    }

    /* Seperate layers */
    for (i = 0; i < 80; i += 20)
    {
        for (j = 0; j < 16; j += 5)
        {
            rooms[i + j + 4].connect[EAST] = NO_ROOM;
            rooms[i + j].connect[WEST] = NO_ROOM;
        }
        for (j = 0; j < 5; j++)
        {
            rooms[i + j].connect[NORTH] = NO_ROOM;
            rooms[i + j + 15].connect[SOUTH] = NO_ROOM;
        }
    }
    /* Connect layers */
    for (i = 0; i < 11; i++)
        rooms[levelcon[i][0]].connect[levelcon[i][1]] = levelcon[i][2];
    /* Blocked routes */
    for (i = 0; i < 83; i++)
        rooms[blocked[i][0]].connect[blocked[i][1]] = NO_ROOM;

    return TRUE;
}

bool SetLivings(living)
Living *living;
{
    int i;

    for (i = 0; i < 21; i++)
    {
        living[i].room = beginroom[i];
        living[i].strike = killhits[i];
        living[i].status = 0;
    }

    return TRUE;
}

bool SetItems(items)
Item *items;
{
    int i;
    char *itemnames[ITEM_COUNT];

    /* LoadStrings heap-allocates the strings it loads! */
    LoadStrings(itemnames, ITEM_COUNT, 'i', 0, FALSE);

    for (i = 0; i < ITEM_COUNT; i++)
    {
        items[i].name = itemnames[i];
        items[i].room = itemroom[i];
        items[i].useableon = workon[i];
    }

    return TRUE;
}


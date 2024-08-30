#include "r136.h"

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
void DiamantStatus();
void ComputerStatus();
void DrakeKopStatus();
bool LavaStatus();
void PapierStatus();

void RoomStatus(progdata)
Progdata *progdata;
{
    char *roomname;
    char *roomdescript;

    GetRoomText(progdata->status.curroom, &roomname, &roomdescript);

    wprintw(mainscr, progdata->strings[YOU_ARE_HERE], roomname);

    if (progdata->status.curroom != 61
        && progdata->status.curroom != 31
        && progdata->status.curroom >= 20
        && !progdata->status.lamp)
    {
        cputs(progdata->strings[TOO_DARK_TO_SEE]);
    }
    else
    {
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

        for (i = 0; i < 25; i++)
            if (progdata->items[i].room == progdata->status.curroom)
            {
                if (wherex() > 55)
                    putch('\n');
                if (progdata->items[i].name == NULL)
                    LoadItemName(i, &(progdata->items[i].name));
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
        return true;

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
        if (progdata->living[STEMMEN].status == 0)
            progdata->living[STEMMEN].status = 1;
        break;
    
    case BARBECUE:
        BarbecueStatus(progdata);
        break;
    
    case BOOM:
        BoomStatus(progdata);
        break;
    
    case DIAMANT:
        DiamantStatus(progdata);
        break;
    
    case COMPUTER:
        ComputerStatus(progdata);
        break;
    
    case DRAKEKOP:
        DrakeKopStatus(progdata);
        break;
    
    case LAVA:
        return LavaStatus(progdata);
    
    case VACUUM:
        progdata->status.lifepoints -= 4; //   Levenswond
        progdata->status.curroom = 76; //   Grot terug
        return false;
    
    case PAPIER:
        PapierStatus(progdata);
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
        return false;
    
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
        return false;
    
    case TELEPORT:
        progdata->status.curroom = 1;
        return false;
    }
    return true;
}

void HellehondStatus(progdata)
Progdata &progdata;
{
    switch (progdata->living[HELLEHOND].status)
    {
    case 1:
        progdata->status.lifepoints--; //  Wond
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
        progdata->status.lifepoints -= 4; //    Grote wond
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
    case 0:
        cputs("Op deze open plek staat een barbecue gezellig te branden.\n\n");

        break;
    case 1:
        cputs("Als je de hasj op de barbecue gooit verschiet de vlam van kleur. Verder gebeurt\n\
er niets.\n\n");

        progdata->living[BARBECUE].status = 3;
        break;
    case 2:
        cputs("Als je het vlees op de barbecue gooit verschiet de vlam van kleur. Verder\n\
gebeurt er niets.\n\n");

        progdata->living[BARBECUE].status = 3;
        break;
    case 3:
        cputs("De barbecue brandt nog steeds, alleen iets onrustiger dan eerst.\n\n");

        break;
    case 4:
      cputs("Een grote rookontwikkeling treedt op wanneer het tweede ingredient in \n\n\
barbecue belandt.\n\
Knetterend smelten de 2 ingredienten om tot een koekje.\n\n");

        progdata->items[KOEKJE].room = progdata->status.curroom;
        progdata->living[BARBECUE].status = 0;
        break;
    }
}

void ApplySimmeringForest(progdata)
Progdata *progdata;
{
    static char *smeulendbos = "Om je heen zie je de smeulende resten van wat eens bos was.";

    for (int i = 0; i < 20; i += 5)
        for (int j = 0; j < 2; j++)
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
    case 0:
        cputs("In een kleine open plek staat een grote, kurkdroge, dode boom. Op de stam zit\n\
een bordje met daarop de tekst \"Roken en open vuur verboden\".\n\n");

        break;
    case 1:
        cputs("Uit de pijp van de vlammenwerper spuit een enorme vlam. De boom begint langzaam\n\
te branden, en weldra staat hij in lichterlaaie. De vlammen slaan om zich heen,\n\
en het hele bos begint mee te branden. Je bent omringd door een enorme vuurzee,\n\
en de hitte is enorm.\n\n");

        if (progdata->items[HITTEPAK].room != OWNED)
        {
            cputs("Je hebt niets om je te beschermen tegen de hitte, en je loopt flinke brandwon-\n\
den op.\n\n");

            progdata->status.lifepoints -= 4; //   Levenswond
        }

        ApplySimmeringForest(progdata);

        progdata->items[GROENKRISTAL].room = 4;
        progdata->living[DIAMANT].status = 1;
        progdata->living[BOOM].status = 2;
        break;
    case 2:
        cputs("Uit de grond steken nog een paar wortels, en er naast ligt een verkoold stuk\n\
bord met daarop \"R   n e  op n v u  ver  d n\".\n\n");

        break;
    }
}

void DiamantStatus(progdata)
Progdata *progdata;
{
    switch (progdata->living[DIAMANT].status)
    {
    case 0:
        cputs("Je struikelt over iets. Door de begroeiing zie je niet wat het is.\n\n");

        break;
    }
}

void ComputerStatus(progdata)
Progdata *progdata;
{
    switch (progdata->living[COMPUTER].status)
    {
    case 0:
        cputs("Er staat een computer van het merk Beull die bestaat bestaat uit een kast met\n\
een 3.5-inch drive en een monitor. Op de monitor staat: \"Datadisk invoeren\n\
a.u.b.\".\n\n");

        progdata->living[COMPUTER].status = 1;
        break;
    case 1:
        cputs("De computer wacht nog steeds.\n\n");

        break;
    case 2:
        cputs("De drive begint te lezen en na korte tijd verschijnt er informatie op het\n\
scherm. Er staat: \"In het onderste grottenstelsel moet men een letter-\n\
route volgen die resulteert in de naam van het te vinden voorwerp.\".\n\
Na even wordt het scherm zwart.\n\n");

        progdata->living[COMPUTER].status = 3;
        break;
    case 3:
        cputs("Er valt niets te zien op de monitor en de computer is stil.\n\n");

        break;
    }
}

void DrakeKopStatus(Progdata &progdata)
{
    switch (progdata->living[DRAKEKOP].status)
    {
    case 0:
        cputs("Er zit in het noorden een zware, dichte deur met daarnaast een drakekop met een\n\
geopende muil. Op de deur zit een zwaar slot.\n\n");

        break;
    case 1:
        cputs("Je stopt het kristal in de muil van de drakekop, die daarop dicht- en weer\n\
opengaat. Het kristal is nu verdwenen, en de ogen van de kop beginnen licht te\n\
gloeien.\n\n");

        progdata->living[DRAKEKOP].status = 2;
        break;
    case 2:
        cputs("De ogen van de draak blijven licht gloeien.\n\n");

        break;
    case 3:
        cputs("Je stopt nog een kristal in de muil. Weer sluit en opent deze, en weer is het\n"
                "kristal verdwenen. Het schijnsel uit de ogen wordt nu sterker.\n\n");

        progdata->living[DRAKEKOP].status = 4;
        break;
    case 4:
        cputs("De ogen van de draak blijven gloeien.\n\n");

        break;
    case 5:
        cputs("Je legt het laatste kristal in de kop. De muil sluit zich weer, en nu blijft\n"
                "hij dicht. De ogen beginnen nu steeds feller te gloeien. Op een gegeven moment\n"
                "concentreert de gloed zich tot een erg felle lichtstraal, die langs je schiet\n"
                "en wordt weerkaatst in een spiegel vlak achter je. De spiegel reflecteert het\n"
                "licht met akelige precisie op het zware slot dat door de enorme hitte verdampt.\n"
                "Daarna zwaait de deur langzaam met veel gepiep open.\n\n");

        progdata->rooms[32].connect[NORTH] = 27;
        progdata->living[DRAKEKOP].status = 6;
        break;
    case 6:
        cputs("De zware deur is nu open en geeft toegang tot een ruimte.\n\n");

        break;
    }
}

bool LavaStatus(Progdata &progdata)
{
    switch (progdata->living[LAVA].status)
    {
    case 0:
        if (progdata->items[HITTEPAK].room == OWNED)
            cputs("Voor je zie je een krater waarin lava opborrelt. Van het lava komt een dikke\n"
                    "damp, en een rode gloed verlicht de omtrek. De hitte is enorm, maar het hitte-\n"
                    "pak beschermt je tegen verbranding.\n\n");
        else
        {
            cputs("Voor je zie je een krater waarin lava opborrelt. De hitte is zo intens, dat je\n"
                    "een aantal brandwonden oploopt en naar achteren wordt geblazen.\n\n");

            progdata->status.curroom = 54; //   Grot terug
            progdata->status.lifepoints -= 4; //   Levenswond
            return false;
        }
        return true;
    case 1:
        cputs("Je gooit de positronenbom met een pisboogje in de lava. Met luid gesis zakt\n"
                "de bom langzaam weg naar beneden. De lava begint op een gegeven moment vreemd\n"
                "te borrelen en verschiet ineens van rood naar groen. Dan zie je een oogver-\n"
                "blindende flits vanuit het lava. Daarna wordt het weer rustiger.\n\n"
                "Het lijkt erop dat je de wereld hebt behoed voor de totaalvernietiging door\n"
                "dit positronenwapen, waarvan de beginselen mede van jouw handen komen. Je mis-\n"
                "sie is voltooid, en vermoeid en vol van gemengde gevoelens verlaat je het\n"
                "grottenstelsel.\n\n");

        ForceExit();
    }
    return true;
}

void PapierStatus(Progdata &progdata)
{
    switch (progdata->living[PAPIER].status)
    {
    case 0:
        cputs("Er zit een dicht, houten luik in het plafond. Je kunt er niet bij.\n\n");

        break;
    case 1:
        cputs("Het luik in het plafond gaat open en er dwarrelt een vel papier naar beneden.\n\n");

        progdata->items[PAPIERITEM].room = 66;
        progdata->living[PAPIER].status = 2;
        break;
    case 2:
        cputs("Het luik aan het plafond hangt nu open. Er zit een leeg gat.\n\n");

        break;
    }
}



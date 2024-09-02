#include "cmd.h"

/* Preloaded strings. 0 to 5 are taken by the directions of navigation. */

#define YOU_CAN_GO_TO            6
#define AND                      7
#define YOU_ARE_HERE             8
#define TOO_DARK_TO_SEE          9
#define ITEM_HERE               10
#define ITEMS_HERE              11
#define SIMMERING_FOREST        12
#define YESNO                   13
#define NEW_OR_SAVED_GAME       14
#define ERROR_READING_DATA      15
#define PRESS_ANY_KEY           16
#define WANT_TO_SAVE_GAME       17
#define ERROR_WRITING_DATA      18
#define COULDNT_OPEN_SAVE_FILE  19
#define STATUS_NOT_SAVED        20
#define WITH                    21
#define TITLE                   22

#define Str(id)             progdata->strings[id]

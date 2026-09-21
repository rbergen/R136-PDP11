/* Key codes returned by getkeypress() */
#define KEY_UP          -1
#define KEY_DOWN        -2
#define KEY_RIGHT       -3
#define KEY_LEFT        -4
#define KEY_HOME        -5
#define KEY_INSERT      -6
#define KEY_DELETE      -7
#define KEY_END         -8
#define KEY_PAGEUP      -9
#define KEY_PAGEDOWN    -10
#define KEY_STAB        -11
#define KEY_ESCAPE      -12
#define KEY_UNHANDLED   -13

/* The most a line in a text file may hold, newline included. files.c sizes
   every line buffer by this, and gendata refuses to build the data files if a
   text exceeds it, so a line that has grown too long stops the build instead
   of being quietly cut short in the game. */
#define TEXT_LINE_LENGTH    256

int vsscanf();
int getkeypress();
void memshift();
void fuzzle();


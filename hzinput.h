/* hzinput.h */
#ifndef __HZINPUT_H__
#define __HZINPUT_H__

#define INPUT_BGCOLOR           8
/* LightBlack */
#define INPUT_FGCOLOR           15
/* LightWhite */
#define INPUT_AREAY          ((18 * 24)+10) 

#define MAX_INPUT_LENGTH        15

#define MAGIC_NUMBER            "CCEGB"
#define CIN_ENAME_LENGTH        24
#define CIN_CNAME_LENGTH        16
#define MAX_PHRASE_LENGTH       20 
#define SELECT_KEY_LENGTH       16 
#define END_KEY_LENGTH          16

#define InputAreaX              16

/* key of toggle input method */

#define NR_INPUTMETHOD  10

typedef struct {
    unsigned long key1; /* sizeof(ITEM=12) */
    unsigned long key2;
    unsigned short ch;
    unsigned short frequency;
} ITEM;

typedef struct {
    char magic_number[sizeof(MAGIC_NUMBER)]; /* magic number */
    char ename[CIN_ENAME_LENGTH]; /* ascii name */
    char cname[CIN_CNAME_LENGTH]; /* prompt */
    char selkey[SELECT_KEY_LENGTH]; /* select keys */

    char last_full; /* last full code need a more SPACE? */
    int TotalKey; /* number of keys needed */
    int MaxPress; /* Max len of keystroke */
    int MaxDupSel; /* how many keys used to select */
    int TotalChar; /* Defined characters */

    unsigned char KeyMap[128]; /* Map 128 chars to 64(6 bit) key index */
    unsigned char KeyName[64]; /* Map 64 key to 128 chars */
    unsigned short KeyIndex[64]; /* 64 key first index of TotalChar */

    int PhraseNum; /* Total Phrase Number */
    FILE *PhraseFile; /* *.tab.phr Phrase File */
    FILE *AssocFile; /* *.tab.lx LianXiang File */
    ITEM *item; /* item */
} hz_input_table;

/************ private functions *******************/
void input_clear_line(int y, int color);
void input_draw_ascii(int x, int y, unsigned char c, int fg, int bg);
void input_print_string(int x, int y, unsigned char *string, int fg, int bg);
void FindMatchKey(void);
void FillMatchChars(int j);
void FillAssociateChars(int index);
void FindAssociateKey(int index);

hz_input_table* IntCode_Init(void);
void IntCode_FindMatchKey(void);
void IntCode_FillMatchChars(int index);
void intcode_hz_filter(int tty_fd, unsigned char key);
/************ public functions *********************/

void hz_input_init(void);
void hz_input_done(void);
void load_input_table(int, unsigned char *);
void unload_input_table(int i);
void hz_filter(int tty_fd, unsigned char c);
void toggle_input_method(void);
void toggle_half_full(void);
void set_active_input_method(int);
void refresh_input_method_area(void);
void DispSelection(void);

extern char *tabfname[10];

#endif
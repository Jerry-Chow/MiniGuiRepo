/* cin2tab.c */
#include <stdio.h>
#include <stdarg.h> 
#include <sys/types.h>
#include <string.h>
#include "hzinput.h"

FILE *fr, *fw;
int lineno;

void print_error(char *fmt, ...) {
    va_list args;

    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
    exit(-1);
}

char *skip_space(char *s) {
    while ((*s == ' ' || *s == '\t') && *s)
        s++;
    return s;
}

char *to_space(char *s) {
    while (*s != ' ' && *s != '\t' && *s)
        s++;
    return s;
}

void del_nl_space(char *s) {
    char *t;

    int len = strlen(s);
    if (!*s)
        return;
    t = s + len - 1;
    while ((*t == '\n' || *t == ' ' || *t == '\t') && s < t)
        t--;
    *(t + 1) = 0;
}

void get_line(u_char *tt) {
    while (!feof(fr)) {
        fgets(tt, 128, fr);
        lineno++;
        if (tt[0] == '#')
            continue;
        else
            break;
    }
}

void cmd_arg(u_char *s, u_char **cmd, u_char **arg) {
    char *t;

    get_line(s);
    if (!*s) {
        *cmd = *arg = s;
        return;
    }

    s = skip_space(s);
    t = to_space(s);
    *cmd = s;
    if (!(*t)) {
        *arg = t;
        return;
    }
    *t = 0;
    t++;
    t = skip_space(t);
    del_nl_space(t);
    *arg = t;
}

typedef struct {
    u_long key1;
    u_long key2;
    u_short ch;
    u_short occur_seq;
} ITEM2;

#define MAXSIZE 50000
/* maximum char/phrase can be defined */

ITEM2 ItemTable[MAXSIZE], ItemTmp[MAXSIZE];
ITEM ItemOut[MAXSIZE];
int PhraseIndex[MAXSIZE];
char PhraseBuffer[250000]; /* max 250K phrase buffer */

int AssocCount[72 * 94 + 1];
int *AssocPhrase[72 * 94];

/* qcmp2 compare two ITEM2 structure, according to its key1/key2/ch */
int qcmp2(ITEM2* a, ITEM2* b) {
    if (a->key1 > b->key1)
        return 1;
    if (a->key1 < b->key1)
        return -1;
    if (a->key2 > b->key2)
        return 1;
    if (a->key2 < b->key2)
        return -1;
    if (a->ch > b->ch)
        return 1;
    if (a->ch < b->ch)
        return -1;
    /*
     fprintf(stderr,"%c%c is multiply defined with the same key\n", 
     a->ch/256,a->ch%256);
     */
    /* duplicate char, we will elimintate it later */
    return 0;
}

/* qcmp compare two ITEM2 structure, according to its key1/key2/occur_seq */
int qcmp(ITEM2* a, ITEM2* b) {
    if (a->key1 > b->key1)
        return 1;
    if (a->key1 < b->key1)
        return -1;
    if (a->key2 > b->key2)
        return 1;
    if (a->key2 < b->key2)
        return -1;
    return (int) a->occur_seq - (int) b->occur_seq;
}

int qcmp_ser(ITEM *a, ITEM* b) {
    if (a->ch > b->ch)
        return 1;
    if (a->ch < b->ch)
        return -1;
    if (a->key1 > b->key1)
        return 1;
    if (a->key1 < b->key1)
        return -1;
    if (a->key2 > b->key2)
        return 1;
    if (a->key2 < b->key2)
        return -1;
    return 0;
}

int main(int argc, char **argv) {
    int i, k;
    char fname[64], fname_cin[64], fname_tab[64];
    char fname_phr[64], fname_lx[64];
    char tt[128];
    u_char *cmd, *arg;
    hz_input_table InpTable;
    int TotalKeyNum;
    ITEM LastItem;
    int index, ItemCount;
    u_short CharDef[64];
    int phrase_count = 0, phrasebuf_pointer = 0;

    if (argc <= 1) {
        printf("Enter table file name [.cin] : ");
        scanf("%s", fname);
    } else
        strcpy(fname, argv[1]);

    strcpy(fname_cin, fname);
    strcpy(fname_tab, fname);
    strcat(fname_cin, ".cin");
    strcat(fname_tab, ".tab");
    strcpy(fname_phr, fname_tab);
    strcpy(fname_lx, fname_tab);
    strcat(fname_phr, ".phr");
    strcat(fname_lx, ".lx");

    if ((fr = fopen(fname_cin, "r")) == NULL)
        print_error("Cannot open %s \n", fname_cin);

    bzero(&InpTable, sizeof(InpTable));
    bzero(ItemTable, sizeof(ItemTable));
    bzero(ItemOut, sizeof(ItemOut));

    printf("Generating binary *.tab file for input method %s...\n", fname_cin);

    /****************** Now some basic information ************************/

    strcpy(InpTable.magic_number, MAGIC_NUMBER);
    cmd_arg(tt, &cmd, &arg);
    if (strcmp(cmd, "%ename") || !(*arg))
        print_error("%d:  %%ename english_name  expected", lineno);
    arg[CIN_ENAME_LENGTH - 1] = 0;
    strcpy(InpTable.ename, arg);

    cmd_arg(tt, &cmd, &arg);
    if (strcmp(cmd, "%prompt") || !(*arg))
        print_error("%d:  %%prompt prompt_name  expected", lineno);
    arg[CIN_CNAME_LENGTH - 1] = 0;
    strcpy(InpTable.cname, arg);

    cmd_arg(tt, &cmd, &arg);
    if (strcmp(cmd, "%selkey") || !(*arg))
        print_error("%d:  %%selkey select_key_list expected", lineno);
    strcpy(InpTable.selkey, arg);

    cmd_arg(tt, &cmd, &arg);
    if (strcmp(cmd, "%last_full") || !(*arg))
        InpTable.last_full = 1;
    else {
        if (arg[0] == '0')
            InpTable.last_full = 0;
        else
            InpTable.last_full = 1;
        cmd_arg(tt, &cmd, &arg);
    }

    if (strcmp(cmd, "%dupsel") || !(*arg))
        print_error("%d:  %%dupsel NO of dup sel keys  expected", lineno);
    InpTable.MaxDupSel = atoi(arg);

    /******************* now the keyname ****************************/

    cmd_arg(tt, &cmd, &arg);
    if (strcmp(cmd, "%keyname") || strcmp(arg, "begin"))
        print_error("%d:  %%keyname begin   expected", lineno);

    TotalKeyNum = 0;
    while (1) {
        cmd_arg(tt, &cmd, &arg);
        if (!strcmp(cmd, "%keyname"))
            break;
        k = tolower(cmd[0]); /* k = char */
        if (InpTable.KeyMap[k])
            print_error("%d:  key %c is already used", lineno, k);

        InpTable.KeyMap[k] = ++TotalKeyNum;

        if (TotalKeyNum > 63)
            print_error("Error, at most 64 key can be defined!\n");
        InpTable.KeyName[TotalKeyNum] = arg[0];
    }

    InpTable.KeyMap[32] = 0; /* SPACE = 32 */
    InpTable.KeyName[0] = ' ';
    TotalKeyNum++;
    InpTable.TotalKey = TotalKeyNum; /* include space */

    /************************ now the character/phrase ***********************/

    cmd_arg(tt, &cmd, &arg);
    index = 0;
    while (!feof(fr)) {
        int len;
        u_long key1, key2;
        int k;

        cmd_arg(tt, &cmd, &arg);
        if (!cmd[0] || !arg[0])
            break;
        len = strlen(cmd);
        if (len > InpTable.MaxPress)
            InpTable.MaxPress = len;
        if (len > 10)
            print_error("%d:  only <= 10 keys is allowed", lineno);

        key1 = 0;
        key2 = 0;
        for (i = 0; i < len; i++) {
            if (i < 5) {
                k = InpTable.KeyMap[cmd[i]];
                key1 |= k << (24 - i * 6);
            } else {
                k = InpTable.KeyMap[cmd[i]];
                key2 |= k << (24 - (i - 5) * 6);
            }
        }
        memcpy(&ItemTable[index].key1, &key1, 4);
        memcpy(&ItemTable[index].key2, &key2, 4);

        /* is it a chinese character(GB), or phrase ? */
        len = strlen(arg);
        if (len == 2 && (int) (*arg) > 0xA0 && (int) (*(arg + 1)) > 0xA0)
            memcpy(&ItemTable[index].ch, arg, 2);
        else {
            int m;
            ItemTable[index].ch = phrase_count;
            /* ch < 0xA1A1, phrase index */
            PhraseIndex[phrase_count] = phrasebuf_pointer;
            strcpy(&PhraseBuffer[phrasebuf_pointer], arg);
            phrasebuf_pointer += len;

            m = (*arg - 0xB0) * 94 + *(arg + 1) - 0xA1;
            if (AssocCount[m] == 0) {
                AssocPhrase[m] = (int*) malloc(sizeof(int) * 5);
            } else if (AssocCount[m] % 5 == 0) {
                AssocPhrase[m] = (int*) realloc(AssocPhrase[m],
                        sizeof(int) * (AssocCount[m] + 5));
            }
            AssocPhrase[m][(AssocCount[m])++] = phrase_count;
            phrase_count++;
        }
        ItemTable[index].occur_seq = index;
        index++;
    }
    fclose(fr);

    if (phrase_count > 0) {
        int i, t, s;
        PhraseIndex[phrase_count++] = phrasebuf_pointer;
        if ((fw = fopen(fname_phr, "w")) == NULL)
            print_error("Cannot create %s\n", fname_phr);
        printf("Phrase Count = %d ", phrase_count);
        fwrite(&phrase_count, 4, 1, fw);
        fwrite(PhraseIndex, 4, phrase_count, fw);
        fwrite(PhraseBuffer, 1, phrasebuf_pointer, fw);
        fclose(fw);

        if ((fw = fopen(fname_lx, "w")) == NULL)
            print_error("Cannot create %s\n", fname_lx);

        s = AssocCount[0];
        AssocCount[0] = 0;
        for (i = 1; i <= 72 * 94; i++) {
            t = AssocCount[i];
            AssocCount[i] = AssocCount[i - 1] + s;
            s = t;
        }
        fwrite(AssocCount, sizeof(AssocCount), 1, fw);
        for (i = 0; i < 72 * 94; i++)
            if (AssocCount[i + 1] - AssocCount[i] > 0) {
                fwrite(AssocPhrase[i], sizeof(int),
                        AssocCount[i + 1] - AssocCount[i], fw);
                free(AssocPhrase[i]);
            }
        fclose(fw);
    }
    InpTable.PhraseNum = phrase_count;
    InpTable.TotalChar = index;
    qsort(ItemTable, index, sizeof(ITEM2), qcmp2);

    /******************** eliminate the dupplicated char *******************/
    bzero(&LastItem, sizeof(ITEM));
    ItemCount = 0;
    for (i = 0; i < index; i++) {
        if (memcmp(&ItemTable[i], &LastItem, sizeof(ITEM))) {
            memcpy(&ItemTmp[ItemCount++], &ItemTable[i], sizeof(ITEM2));
            memcpy(&LastItem, &ItemTable[i], sizeof(ITEM));
        }
    }

    printf("Total Item = %d\n\n", ItemCount);
    index = ItemCount;
    qsort(ItemTmp, index, sizeof(ITEM2), qcmp);
    /* sorting the char/phrase according its key & appearance ordr */

    /* now eliminate the occurance field , ITEM2->ITEM */
    for (i = 0; i < index; i++) {
        memcpy(&ItemOut[i], &ItemTmp[i], sizeof(ITEM));
        /*
         if (i%100 == 0)
         printf("No %d: key1=%08X key2=%08X Char=%04X\n",
         i, ItemOut[i].key1, ItemOut[i].key2, ItemOut[i].ch);
         */
    }

    /******************* generate 64 index number ***********************/

    bzero(CharDef, sizeof(CharDef));
    for (i = 0; i < index; i++) {
        int kk = (ItemOut[i].key1 >> 24) & 0x3f;
        if (!CharDef[kk]) {
            InpTable.KeyIndex[kk] = (u_short) i;
            CharDef[kk] = 1;
        }
    }

    InpTable.KeyIndex[TotalKeyNum] = index;
    for (i = TotalKeyNum - 1; i > 0; i--)
        if (!CharDef[i])
            InpTable.KeyIndex[i] = InpTable.KeyIndex[i + 1];

    /*
     for(i = 0; i < 64; i++)
     printf("Index %d = %d\n",i,InpTable.KeyIndex[i]);
     */

    if ((fw = fopen(fname_tab, "w")) == NULL) {
        print_error("Cannot create");
    }

    fwrite(&InpTable, 1, sizeof(InpTable), fw);
    fwrite(ItemOut, sizeof(ITEM), index, fw);
    fclose(fw);

    return 0;
}

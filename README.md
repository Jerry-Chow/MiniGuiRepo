# MiniGuiRepo

MiniGui输入法插件支持日语；MiniGui日本語入力対応；MiniGui input method plug-in supports Japanese



*MiniGUI* is a modern and mature cross-platform window system for embedded and smart IoT devices. Here I want to share some resource to solve the issue of supporting Japanese input method in MiniGui.



MiniGUI是一个现代的、成熟的跨平台窗口系统，用于嵌入式和智能物联网设备。最近要使 MiniGui 支持日语，所以共享一些找到的资料，以方便后来者。



*MiniGUI*は、組み込み機器やスマートIoTデバイスのためのモダンで成熟したクロスプラットフォームウィンドウシステムです。ここでは、MiniGuiで日本語入力メソッドをサポートするための問題を解決するためのリソースを紹介したいと思います。



首先，MiniGui的词库比较特别，需要转换；其次，中文词库好找日语的词库不好找，我这里也打算分享一些。



First of all, MiniGui's input words talbe is rather special and needs to be converted; secondly,  Japanese convert table is not easy to find, and I intend to share some of them here.



まず、MiniGuiの入力語talbeはかなり特別で、変換する必要がある。次に、日本語の変換表はなかなか見つからないので、ここでその一部を紹介するつもりである。

前言
1. 词库文件构成
2. 词库文件生成
2.1 pinyin.cin 修改
2.2 pinyin.tab 生成
2.3 ime_tab_pinyin.h 生成
3. libmgi 中使用新词库
4. 附录

MiniGUI 输入法词库更新 CSDN 博客
前言
MiniGUI 中的输入法 libmgi 支持拼音输入，但是怎么进行个性化定制呢，比如想更换键盘皮肤，往词库里添加一些新的词组，下面这篇文章详细描述怎么添加新词

1. 词库文件构成
在 libmgi 中，拼音输入法词库文件是如下路径

/libmgi-2.0.4/src/ime_tab_pinyin.h
加载和使用这词库的代码

/libmgi-2.0.4/src/ime_pinyin.c
在 minigui-res-1.6.10 中有找到一些词库文件

pinyin.cin		//拼音输入法词库文件
pinyin.map		//拼音输入法支持的汉字
pinyin.tab		//会保存有多少个词，词频等信息
pinyin.tab.phr	//是短语文件
pinyin.tab.lx	//是联想文件
这里捋一下关系
pinyin.cin–>(pinyin.tab、pinyin.tab.phr、pinyin.tab.lx)–>ime_tab_pinyin.h

主要的问题是有了 pinyin.cin 怎么生成 pinyin.tab、pinyin.tab.lx、pinyin.tab.phr

2. 词库文件生成
首先需要去 SourceForge 下载 CCE 软件的源码 cce-0.11，我也上传了一份到 CSDN 备份，点击下载 cce-0.11，这个软件比较古老，不知道什么时候就会下载不了，且行且珍惜

下载之后解压得到 cce-0.11 文件夹，在 / cce-0.11/input_methods / 目录下会看到这几个文件

/cce-0.11/input_methods/cin2tab.c	//生成pinyin.tab、pinyin.tab.lx、pinyin.tab.phr的程序
/cce-0.11/input_methods/hzinput.h	//编译cin2tab.c的头文件
/cce-0.11/input_methods/pinyin.cin	//拼音输入法词库文件
/cce-0.11/input_methods/Makefile
2.1 pinyin.cin 修改
词库文件是 pinyin.cin，打开可以看到排列的一些规律，注意文件编码格式是 GB2312 的
按照字母排序添加一行需要的新词，比如添加美食，如下所示

meishao 眉梢
meishi 每时
meishi 美食
meishu 美术
meishuo 媒妁
2.2 pinyin.tab 生成
按照下面的补丁修改 / cce-0.11/input_methods/Makefile 文件，目的是为了编译成 32 位可执行程序，不然生成的词库在 MiniGUI 中用不了

diff --git a/input_methods/Makefile b/input_methods/Makefile
index 296f70a..d413709 100644
--- a/input_methods/Makefile
+++ b/input_methods/Makefile
@@ -22,16 +22,16 @@ ziranma.tab ziranma.tab.lx ziranma.tab.phr: cin2tab ziranma.cin
        ./cin2tab ziranma
 
 cin2tab: cin2tab.c
-       gcc -o cin2tab cin2tab.c
+       gcc -m32 -o cin2tab cin2tab.c
 
 tab2cin: tab2cin.c
-       gcc -o tab2cin tab2cin.c
+       gcc -m32 -o tab2cin tab2cin.c
 
 tab2dat: tab2dat.c
-       gcc -o tab2dat tab2dat.c
+       gcc -m32 -o tab2dat tab2dat.c
 
 cin2dat: cin2dat.c
-       gcc -o cin2dat cin2dat.c
+       gcc -m32 -o cin2dat cin2dat.c
 
 clean:
        rm -f tab2cin cin2tab tab2dat cin2dat
在 / cce-0.11/input_methods / 目录下，执行下面的命令

make clean && make
生成 cin2tab 可执行程序之后，在 / cce-0.11/input_methods / 目录下，执行下面命令

./cin2tab ./pinyin
看到下面的提示就表示成功了

Generating binary *.tab file for input method ./pinyin.cin...
Phrase Count = 13278 Total Item = 20942
会生成下面的文件，大功完成一半

/cce-0.11/input_methods/pinyin.tab		//会保存有多少个词，词频等信息
/cce-0.11/input_methods/pinyin.tab.phr	//是短语文件
/cce-0.11/input_methods/pinyin.tab.lx	//是联想文件
2.3 ime_tab_pinyin.h 生成
下面提供了生成 ime_tab_pinyin.h 的 JAVA 程序使用方法和源码

把 pinyin.tab、pinyin.tab.lx、pinyin.tab.phr 复制到一个目录下，使用 TabBytesToC.jar 进行转换

TabBytesToC.jar 程序可以把指定目录下的. tab、.tab.lx 和. tab.phr 三个文件换成十六进制的头文件表示
请指定文件所在的目录，例如
java -jar TabBytesToC.jar /home/anruliu / 下载 / tab/

输出如下

anruliu@anruliu:~/下载$ java -jar TabBytesToC.jar /home/anruliu/下载/tab/
该目录下文件个数：3
文件：/home/anruliu/下载/tab/pinyin.tab
文件：/home/anruliu/下载/tab/pinyin.tab.lx
文件：/home/anruliu/下载/tab/pinyin.tab.phr
pinyin.tab 字节长度：251720 正在解析中...
static unsigned char ime_tab[] 生成成功
pinyin.tab.lx 字节长度：80184 正在解析中...
static unsigned char ime_tab_lx[] 生成成功
pinyin.tab.phr 字节长度：110468 正在解析中...
static unsigned char ime_tab_phr[] 生成成功
/home/anruliu/下载/tab/ime_tab_pinyin.h 生成成功
下面是源码，可以编译出 TabBytesToC.jar 程序
    ''
    import java.io.File;
    import java.io.FileInputStream;
    import java.io.FileWriter;
    import java.io.IOException;
    import java.util.Arrays;
    import java.util.Collections;
    import java.util.Comparator;
    import java.util.List;

    /**
    * CCE生成的tab词库文件转成十六进制libmgi可用的头文件
    */
    public class TabBytesToC {

        public static void main(String[] args) throws Exception {
            if (args.length > 0) {
                File file = new File(args[0]);
                if (file.exists()) {
                    if (file.isFile()) {
                        System.out.println("请指定目录，而不是文件");
                    } else {
                        File[] tempList = file.listFiles();
                        // 文件排序
                        List<File> fileList = Arrays.asList(tempList);
                        Collections.sort(fileList, new Comparator<File>() {
                            @Override
                            public int compare(File o1, File o2) {
                                if (o1.isDirectory() && o2.isFile())
                                    return -1;
                                if (o1.isFile() && o2.isDirectory())
                                    return 1;
                                return o1.getName().compareTo(o2.getName());
                            }
                        });
                        if (tempList.length > 0)
                            System.out.println("该目录下文件个数：" + tempList.length);
                        for (int i = 0; i < tempList.length; i++) {
                            if (tempList[i].isFile()) {
                                System.out.println("文件：" + tempList[i]);
                            }
                            if (tempList[i].isDirectory()) {
                                System.out.println("文件夹：" + tempList[i]);
                            }
                        }

                        // 如果需要生成的.h文件存在，则先删除
                        File fileOut = new File(args[0] + "/ime_tab_pinyin.h");
                        if (fileOut.exists())
                            fileOut.delete();

                        for (int i = 0; i < tempList.length; i++) {
                            if (tempList[i].isFile()) {
                                tab2Txt(tempList[i]);
                            }
                        }
                        if (tempList.length > 0)
                            System.out.println(args[0] + "ime_tab_pinyin.h 生成成功");
                    }
                }
            } else {
                System.out.println("本程序可以把指定目录下的.tab、.tab.lx和.tab.phr"
                        + "三个文件换成十六进制的头文件表示");
                System.out.println("请指定文件所在的目录，例如");
                System.out.println("java -jar TabBytesToC.jar /home/anruliu/pinyin/");
            }
        }

        private static void tab2Txt(File file) {
            String ext = file.getName().substring(file.getName().lastIndexOf(".") + 1);
            if (ext.equals("tab") || ext.equals("lx") || ext.equals("phr")) {
                try {
                    FileInputStream fis = new FileInputStream(file);
                    String fileNameData = new String();
                    if (ext.equals("tab")) {
                        fileNameData = "ime_tab";
                    } else if (ext.equals("lx")) {
                        fileNameData = "ime_tab_lx";
                    } else if (ext.equals("phr")) {
                        fileNameData = "ime_tab_phr";
                    }
                    java.io.ByteArrayOutputStream bos = new java.io.ByteArrayOutputStream();
                    byte[] buff = new byte[1024];
                    int len = 0;

                    while ((len = fis.read(buff)) != -1) {
                        bos.write(buff, 0, len);
                    }

                    // 得到图片的字节数组
                    byte[] result = bos.toByteArray();
                    System.out.println(file.getName() + " 字节长度：" + result.length + " 正在解析中...");

                    if (null != file.getParent()) {
                        byte2HexStr(file.getParent(), "ime_tab_pinyin", fileNameData, result);
                    } else {
                        byte2HexStr(".", "ime_tab_pinyin", fileNameData, result);
                    }

                    fis.close();
                    bos.close();

                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
        }

        /**
        * 实现字节数组向十六进制转换并保存
        * 
        * @param b 字节数组
        * @return 十六进制字符串
        */
        private static void byte2HexStr(String filePatch, String fileName,
                String fileNameData, byte[] b) {
            String hs = "  ";
            String stmp = "";
            FileWriter fwriter = null;
            try {
                fwriter = new FileWriter(filePatch + "/" + fileName + ".h", true);
                fwriter.write("static unsigned char " + fileNameData + "[] = {\n");
                for (int n = 0; n < b.length; n++) {
                    System.out.print(n + "/" + b.length + "\r");
                    stmp = (Integer.toHexString(b[n] & 0XFF));
                    if (stmp.length() == 1) {
                        hs = hs + "0x0" + stmp;
                    } else {
                        hs = hs + "0x" + stmp;
                    }
                    if (n == b.length - 1) {
                        hs += "\n";
                    } else if (n != 0 && (n + 1) % 8 == 0) {
                        hs += ",\n  ";
                    } else {
                        hs += ", ";
                    }
                    fwriter.write(hs);
                    hs = "";
                }
                fwriter.write("};\n\n");
            } catch (IOException ex) {
                ex.printStackTrace();
            } finally {
                try {
                    fwriter.flush();
                    fwriter.close();
                    System.out.println("static unsigned char " + fileNameData + "[] 生成成功");
                } catch (IOException ex) {
                    ex.printStackTrace();
                }
            }
        }
    }''
3. libmgi 中使用新词库
把新的 ime_tab_pinyin.h 文件替换 / libmgi-2.0.4/src/ime_tab_pinyin.h，然后重新编译 libmgi，如果没有生效，清理一下编译目录，之后就可以看到效果了，如下所示


4. 附录
避免下载不到 cee-0.11，这附上 cin2tab.c 和 hzinput.h 的源码，执行下面的代码编译即可

gcc -m32 -o cin2tab cin2tab.c
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



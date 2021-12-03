/**
 * @file recoveryWalker.c
 */
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "param.h"

char *padTrim(const char *ps, int *count) {
    const char *s = ps;
    for (; *s; s++) {
        *count = *count + 1;
        if (*s != ' ')
            return (char *)s;
    }
    return 0;
}

typedef struct TBnode {
    int type;
    int inum;
    int size;
    struct TBnode *next;
    struct TBnode *prev;
} TBnode;

static TBnode headTBnode = {-1, -1, -1, &headTBnode, &headTBnode};
static TBnode *TBnodes = &headTBnode;

void newTBnode(int type, int inum, int size) {
    TBnode *newNode = malloc(sizeof(TBnode));
    newNode->type = type;
    newNode->inum = inum;
    newNode->size = size;

    newNode->next = TBnodes->next;
    newNode->prev = TBnodes;

    TBnodes->next->prev = newNode;
    TBnodes->next = newNode;

    TBnodes = TBnodes->next;
}

void TBnodeLine(char *line) {
    int i = 0;
    int n[3];

    char *last = line;

    while (i < 3) {
        if (*line == ' ' || *line == '\000') {
            *line = '\000';
            n[i++] = atoi(last);
            last = ++line;
        }
        line++;
    }

    if (n[0] == T_DIR || n[0] == T_FILE)
        newTBnode(n[0], n[1], n[2]);
}

void populateTBnodes(int TBOut) {
    char tBuf[512];
    int tn = 1;
    int tc = 0;

    int t = 0;

    while (1) {

        if (!t && tn > 0) {
            tn = read(TBOut, tBuf + tc, 1);
        }

        if (tn <= 0) {
            break;
        }

        t = tBuf[tc] == '\n';

        tc += !t;

        if (t) {
            t = 0;
            tBuf[tc] = 0;
            tc = 0;

            TBnodeLine(tBuf);
        }
    }

    if (tn < 0) {
        printf(2, "read error\n");
        exit();
    }

    TBnodes = headTBnode.next; // Point to first element
}

void walk(int dirOut, int TBOut) {
    populateTBnodes(TBOut);

    char dBuf[512];
    int dn = 1;
    int dc = 0;
    int d = 0;

    while (1) {
        if (!d && dn > 0) {
            dn = read(dirOut, dBuf + dc, 1);
        }

        if (dn <= 0) {
            break;
        }

        d = dBuf[dc] == '\n';

        dc += !d;

        if (d) {
            d = 0;
            dBuf[dc] = 0;
            dc = 0;

            int level = 0;
            char *dir_line = padTrim(dBuf, &level);

            printf(1, "%d %s\n", level, dir_line);
        }
    }

    if (dn < 0) {
        printf(2, "read error\n");
        exit();
    }
}

int main(int argc, char *argv[]) {

    printf(1, "Storing walker outputs\n");

    if (fork() == 0) {
        close(1);
        open("dOut", O_CREATE | O_WRONLY);
        char *argd[MAXARG] = {"directoryWalker", "-s"};
        exec(argd[0], argd);
        printf(2, "exec directoryWalker failed\n");
        exit();
    }

    if (fork() == 0) {
        close(1);
        open("tOut", O_CREATE | O_WRONLY);
        char *argt[MAXARG] = {"inodeTBWalker"};
        exec(argt[0], argt);
        printf(2, "exec inodeTBWalker failed\n");
        exit();
    }

    wait();
    wait();

    printf(1, "Run recovery walk\n");

    int dirOut = open("dOut", O_RDONLY);
    int TBOut = open("tOut", O_RDONLY);

    walk(dirOut, TBOut);

    close(dirOut);
    close(TBOut);

    exit();
}

/**
 * @file directoryWalker.c
 */

#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"

#define MAX_LEVEL 16

int simple_output = 0;
int level = 0;
int clevel = 0;

char sidePipe[MAX_LEVEL * 4];
char pipeLvlStack[MAX_LEVEL];

char *fmtname(char *path, int pad) {
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    pad &= !simple_output;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), pad ? ' ' : '\00', DIRSIZ - strlen(p));
    return buf;
}

typedef struct dirChain {
    uint inum;
    uint size;
    short type;
    int nlen;
    char name[DIRSIZ];
    struct dirChain *prev;
    struct dirChain *next;
} dirChain;

void upLevel(int cont) {
    if (simple_output) {
        strcpy(sidePipe + clevel, " ");
        clevel += 1;
        pipeLvlStack[level] = 1;
    } else if (cont) {
        strcpy(sidePipe + clevel, "│ ");
        clevel += 4;
        pipeLvlStack[level] = 4;
    } else {
        strcpy(sidePipe + clevel, "  ");
        clevel += 2;
        pipeLvlStack[level] = 2;
    }
    level++;
}

void downLevel() {
    level--;
    clevel -= pipeLvlStack[level];
    sidePipe[clevel] = 0;
}

void printLevelSpace(int alt, int file) {
    if (level > 0 || alt) {
        printf(1, "%s", sidePipe);
        if (!simple_output) {
            if (alt) {
                if (file > 0)
                    printf(1, "└--- ");
                else
                    printf(1, "└ ");
            } else {
                if (file > 0)
                    printf(1, "├--- ");
                else
                    printf(1, "├ ");
            }
        }
    }
}

void walk(char *path);

void printStat(char *fbuf, uint type, uint inum, uint size) {
    printf(1, simple_output ? "%s %d %d %d\n" : "%s type: %d inode: %d size: %d\n", fbuf, type, inum, size);
}

int ls(char *path) {
    char buf[strlen(path) + 1 + DIRSIZ + 1], *p;
    int fd;
    struct dirent de;
    struct stat st;

    dirChain *chain = malloc(sizeof(dirChain)); // FIXME: Page fault occurs often with bigger trees, using too much memory?

    chain->next = chain;
    chain->prev = chain;

    dirChain *headChain = chain;
    dirChain *newChain;

    if ((fd = open(path, 0)) < 0) {
        printf(2, "dirWalk: cannot open %s\n", path);
        return -1;
    }

    if (fstat(fd, &st) < 0) {
        printf(2, "dirWalk: cannot stat %s\n", path);
        close(fd);
        return -1;
    }

    switch (st.type) {
    case T_FILE:
        printf(1, "Use on a directory to walk\n");
        break;

    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
            printf(1, "dirWalk: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        int pfile = 0;
        char fbuf[DIRSIZ];

        while (read(fd, &de, sizeof(de)) == sizeof(de)) {

            if (pfile) {
                printLevelSpace(0, 1);
                printStat(fbuf, st.type, st.ino, st.size);
                pfile = 0;
            }

            if (de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;

            if (stat(buf, &st) < 0) {
                printf(1, "dirWalk: cannot stat %s\n", buf);
                continue;
            }

            switch (st.type) {
            case T_FILE:
                pfile = 1;
                memmove(fbuf, fmtname(buf, 1), DIRSIZ);
                break;

            case T_DIR:
                if (strcmp(fmtname(buf, 0), ".") && strcmp(fmtname(buf, 0), "..")) { // Ignore system dirs
                    newChain = malloc(sizeof(dirChain));
                    newChain->type = st.type;
                    newChain->size = st.size;
                    newChain->inum = st.ino;
                    newChain->nlen = strlen(fmtname(buf, 0));
                    memmove(newChain->name, fmtname(buf, 1), DIRSIZ); // might overread

                    newChain->next = chain->next;
                    newChain->prev = chain;

                    chain->next->prev = newChain;
                    chain->next = newChain;

                    chain = chain->next;
                }
            }
        }
        if (pfile) { // Bottom └ for file not printed at highest node directory?
            printLevelSpace(chain == headChain, 1);
            printStat(fbuf, st.type, st.ino, st.size);
        }
        break;
    }
    close(fd);
    ;
    chain = headChain->next; // Start from beginning
    while (chain != headChain) {
        if (chain->next == headChain)
            printLevelSpace(1, 0);
        else
            printLevelSpace(0, 0);
        printStat(chain->name, chain->type, chain->inum, chain->size);
        memset(buf, 0, sizeof(buf));
        strcpy(buf, path);
        strcpy(buf + strlen(path), "/");
        memmove(buf + strlen(path) + 1, chain->name, chain->nlen);
        upLevel(chain->next != headChain);
        walk(buf);
        downLevel();
        chain = chain->next;
        free(chain->prev);
    }
    free(headChain);

    return 0;
}

void walk(char *path) {
    if (fork() == 0) {
        ls(path);
        exit();
    } else {
        wait();
    }
}

int main(int argc, char *argv[]) {
    int i = 1;

    if (argc > 1 && !strcmp(argv[1], "-s")) {
        simple_output = 1;
        i++;
    }

    if (argc < 2 || (simple_output & (argc < 3))) {
        if (simple_output) {
            ls(".");
            exit();
        }
        printf(1, "  .\n");
        upLevel(0);
        ls(".");
        downLevel();
        exit();
    }

    for (; i < argc; i++) {
        printf(1, "  %s\n", argv[i]);
        upLevel(0);
        ls(argv[i]);
        downLevel();
    }

    exit();
}
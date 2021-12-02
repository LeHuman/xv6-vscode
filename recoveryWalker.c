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

void cmp(char *dir_line, char *TB_line) {
    int level = 0;
    dir_line = padTrim(dir_line, &level);

    printf(1, "%d %s %s\n", level, dir_line, TB_line);
}

void walk(int dirOut, int TBOut) {
    char dBuf[512];
    char tBuf[512];
    int dn = 1, tn = 1;
    int dc = 0, tc = 0;

    int d = 0, t = 0;

    while (1) {
        if (!d && dn > 0) {
            dn = read(dirOut, dBuf + dc, 1);
            // printf(1, "%d\n", dn);
        }

        if (!t && tn > 0) {
            tn = read(TBOut, tBuf + tc, 1);
            // printf(1, "%d\n", tn);
        }

        if (dn <= 0 || tn <= 0) {
            break;
        }

        d = dBuf[dc] == '\n';
        t = tBuf[tc] == '\n';

        dc += !d;
        tc += !t;

        if (d && t) {
            d = 0;
            t = 0;

            dBuf[dc] = 0;
            dc = 0;
            tBuf[tc] = 0;
            tc = 0;

            cmp(dBuf, tBuf);
        }
    }

    if (dn < 0 || tn < 0) {
        printf(1, "read error\n");
        exit();
    }
}

int main(int argc, char *argv[]) {

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

    printf(1, "Run Recovery Walk\n");

    int dirOut = open("dOut", O_RDONLY);
    int TBOut = open("tOut", O_RDONLY);

    walk(dirOut, TBOut);

    close(dirOut);
    close(TBOut);

    exit();
}

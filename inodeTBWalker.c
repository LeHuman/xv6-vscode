/**
 * @file inodeTBWalker.c
 */
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "param.h"

int main(int argc, char *argv[]) {
    int inum = 2;
    struct dinode dip;

    while (1) {
        if (getdiNode(1, inum++, &dip) != 0)
            break;
        switch (dip.type) {
        case T_FILE:
        case T_DIR:
            printf(1, "%d %d %d\n", dip.type, inum - 1, dip.size);
            break;
        case T_DEV:
        default:
            break;
        }
    }
    exit();
}

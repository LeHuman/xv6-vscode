#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "param.h"

int main(int argc, char *argv[]) {
    int inum = 1;
    struct dinode dip;

    int last = -1;
    int lst = 0;

    while (1) {
        if (getdiNode(1, inum++, &dip) != 0)
            break;
        switch (dip.type) {
        case T_FILE:
            if (last != T_FILE) {
                printf(1, "\nfiles: ");
                last = T_FILE;
                lst = 0;
            }
            printf(1, "%d s:%d ", inum, dip.size);
            lst++;
            if (lst == 4) {
                lst = 0;
                printf(1, "\n");
            }
            break;
        case T_DIR:
            if (last != T_DIR) {
                printf(1, "\ndirs: ");
                last = T_DIR;
                lst = 0;
            }
            printf(1, "%d s:%d ", inum, dip.size);
            lst++;
            if (lst == 4) {
                lst = 0;
                printf(1, "\n");
            }
            break;
        case T_DEV:
            if (last != T_DEV) {
                printf(1, "\ndevs: ");
                last = T_DEV;
                lst = 0;
            }
            printf(1, "%d s:%d ", inum, dip.size);
            lst++;
            if (lst == 4) {
                lst = 0;
                printf(1, "\n");
            }
            break;
        default:
            if (dip.size != 0) {
                if (last != 0) {
                    printf(1, "\nFree nodes: ");
                    last = 0;
                    lst = 0;
                }
                printf(1, "%d s:%d a:%d", inum, dip.size);
                lst++;
                if (lst == 4) {
                    lst = 0;
                    printf(1, "\n");
                }
            }
            break;
        }
    }
    printf(1, "\n");
    exit();
}

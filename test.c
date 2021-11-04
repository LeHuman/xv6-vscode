#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    char *shp = GetSharedPage(0, 3);

    strcpy(shp, "Hello!");
    if (fork() == 0) {
        char *shpc = GetSharedPage(0, 3);
        printf(1, "%s\n", shpc);
        memset(shpc, 0, 7);
        while (shpc[0] == 0) {
        }
        printf(1, "%s\n", shpc);
        printf(1, "%p %d\n", shpc, FreeSharedPage(0));
        exit();
    }
    while (shp[0] != 0) {
    }
    strcpy(shp, "Noice!");
    wait();
    printf(1, "%p %d\n", shp, FreeSharedPage(0));
    exit();
}

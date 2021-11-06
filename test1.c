#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    char *shpc = GetSharedPage(0, 3);
    while (shpc[0] == 0) {
    }
    printf(1, "%s\n", shpc);
    memset(shpc, 0, 7);
    while (shpc[0] == 0) {
    }
    printf(1, "%s\n", shpc);
    printf(1, "%p %d\n", shpc, FreeSharedPage(0));
    exit();
}

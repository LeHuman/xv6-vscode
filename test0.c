#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    char *tx = GetSharedPage(0, 6);
    char *rx = GetSharedPage(6, 6);

    while (tx[0] != 0) {
    }
    strcpy(tx, "0: Hello!");

    while (rx[0] == 0) {
    }
    printf(1, "0 Received: %s\n", rx);
    memset(rx, 0, 4096);

    while (tx[0] != 0) {
    }
    strcpy(tx, "0: says Goodbye!");

    while (rx[0] == 0) {
    }
    printf(1, "0 Received: %s\n", rx);
    memset(rx, 0, 4096);

    printf(1, "0 %p %d\n", tx, FreeSharedPage(0));
    printf(1, "0 %p %d\n", rx, FreeSharedPage(6));
    exit();
}

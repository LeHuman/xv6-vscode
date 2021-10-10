#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    int fd = 1337;
    int s = close(fd);
    printf(1, "fd: %d stat: %d\n", fd, s);
    exit();
}

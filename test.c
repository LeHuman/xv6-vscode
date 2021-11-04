#include "types.h"
#include "stat.h"
#include "user.h"

int main(int argc, char *argv[]) {
    // void *shp = GetSharedPage(0, 3);
    void *shp = malloc(512);
    printf("%p\n", shp);
    exit();
}

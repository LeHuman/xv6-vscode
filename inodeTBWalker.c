#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "param.h"

int main(int argc, char *argv[]) {
    struct inode *nodes;
    listNodes(1, nodes);
    exit();
}

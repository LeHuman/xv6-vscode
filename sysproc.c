#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int sys_fork(void) {
    return fork();
}

int sys_exit(void) {
    exit();
    return 0; // not reached
}

int sys_wait(void) {
    return wait();
}

int sys_kill(void) {
    int pid;

    if (argint(0, &pid) < 0)
        return -1;
    return kill(pid);
}

int sys_getpid(void) {
    return myproc()->pid;
}

int sys_sbrk(void) {
    int addr;
    int n;

    if (argint(0, &n) < 0)
        return -1;
    addr = myproc()->sz;
    if (growproc(n) < 0)
        return -1;
    return addr;
}

int sys_sleep(void) {
    int n;
    uint ticks0;

    if (argint(0, &n) < 0)
        return -1;
    acquire(&tickslock);
    ticks0 = ticks;
    while (ticks - ticks0 < n) {
        if (myproc()->killed) {
            release(&tickslock);
            return -1;
        }
        sleep(&ticks, &tickslock);
    }
    release(&tickslock);
    return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int sys_uptime(void) {
    uint xticks;

    acquire(&tickslock);
    xticks = ticks;
    release(&tickslock);
    return xticks;
}

#define MAX_SH_PAGES NPROC
#define MAX_SH_KEY NPROC

int sharedPagesRefs[MAX_SH_KEY] = {0};
void *sharedPages[MAX_SH_KEY][MAX_SH_PAGES] = {0}; // Keys range from 1-NPROC => 1-63 | Maximum of NPROC pages can be shared by each process

void *growprocsh(int k, int n) {

    if (k < 0 || k > MAX_SH_KEY || n < 0 || n > MAX_SH_PAGES) { // Must be within set limits to keep implement simple
        return (void *)-1;
    }

    struct proc *currProc = myproc(); // TODO: resize, multiple keys

    if (sharedPagesRefs[k] == 0) { // New shared page
        void *mem;
        for (int i = 0; i < n; i++) {
            mem = kalloc(); // Physical memory
            if (mem == 0) {
                cprintf("allocuvm out of memory\n");
                return (void *)-1;
            }
            memset(mem, 0, PGSIZE);

            currProc->shaddr -= PGSIZE; // Grow shared memory down from KERNBASE

            // Map virtual page to physical page
            if (mappages(currProc->pgdir, currProc->shaddr, PGSIZE, PADDR(mem), PTE_P | PTE_W | PTE_U) < 0) {
                return (void *)-1;
            }
        }
    } else {
    }
    sharedPagesRefs[k]++;
}

void *sys_GetSharedPage(void) {
    int key, count;

    if (argint(0, &key) < 0 || argint(1, &count) < 0)
        return (void *)-1;

    return growprocsh(key, count);
}

int sys_FreeSharedPage(void) {
    int key;
    if (argint(0, &key) < 0)
        return -1;
    // TODO: Free page Here
    return 0;
}
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

sharedRegion_t sharedRegions[MAX_SH_KEY]; // Keys range from 0-NPROC => 0-63

void *mapSharedRegion(struct proc *p, int key) {
    sharedRegion_t *region = sharedRegions + key; // get region we are intrested in
    sharedReference_t *ref = p->shared + key;

    if (ref->region->valid) // If this region has already been mapped to this process return the mapped virtual address
        return ref->va;

    if (p->shaddr == 0) { // Init shared address to KERNBASE // TODO: Do this when init proc
        p->shaddr = KERNBASE;
    }

    // Starting from KERNBASE or shaddr, subtract `PGSIZE` from current shared region addr (shaddr), then map using this addr. repeat `pageCount` times
    for (int k = 0; k < region->pageCount; k++) {
        if (mappages(p->pgdir, (p->shaddr -= PGSIZE), PGSIZE, V2P(region->pages[k]), PTE_W | PTE_U) < 0) {
            return (void *)-1;
        }
    }

    ref->region = region;         // Set the reference to the region
    return (ref->va = p->shaddr); // Set the reference va to the current shared pointer and return it
}

void *growprocsh(int key, int count) {

    if (key < 0 || key > MAX_SH_KEY || count <= 0 || count > MAX_SH_PAGES) { // Must be within set limits to keep implement simple
        return (void *)-1;
    }

    struct proc *currProc = myproc();             // Get current process
    sharedRegion_t *region = sharedRegions + key; // Get pointer to the shared region that is being requested

    if (!region->valid) {                 // New shared page
        for (int i = 0; i < count; i++) { // Create `count` number of pages
            void *mem = kalloc();         // alloc mem
            if (mem == 0) {               // Error
                cprintf("allocuvm out of memory\n");
                return (void *)-1;
            }
            memset(mem, 0, PGSIZE); // Clear memory
            region->pages[i] = mem; // Save new page to region
        }
        region->valid = 1;                  // region is now valid
        region->refCount = 0;               // reset ref counter
        region->pageCount = count;          // set the size of this region
    } else if (count > region->pageCount) { // Resizing not supported, though returning a larger space than requested should be fine
        return (void *)-1;
    }

    region->refCount++;

    return mapSharedRegion(currProc, key);
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
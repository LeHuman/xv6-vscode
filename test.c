#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

// http://www.iro.umontreal.ca/~simardr/rng/lfsr113.c
unsigned int rand(void) {
    static unsigned int z1 = 12345, z2 = 12345, z3 = 12345, z4 = 12345;
    unsigned int b;
    b = ((z1 << 6) ^ z1) >> 13;
    z1 = ((z1 & 4294967294U) << 18) ^ b;
    b = ((z2 << 2) ^ z2) >> 27;
    z2 = ((z2 & 4294967288U) << 2) ^ b;
    b = ((z3 << 13) ^ z3) >> 21;
    z3 = ((z3 & 4294967280U) << 7) ^ b;
    b = ((z4 << 3) ^ z4) >> 12;
    z4 = ((z4 & 4294967168U) << 13) ^ b;
    return (z1 ^ z2 ^ z3 ^ z4);
}

int randr(int min, int max) {
    return min + (rand() % (int)(max - min + 1));
}

//https://www.techiedelight.com/implement-itoa-function-in-c/

// Function to swap two numbers
void swap(char *x, char *y) {
    char t = *x;
    *x = *y;
    *y = t;
}

// Function to reverse `buffer[i…j]`
char *reverse(char *buffer, int i, int j) {
    while (i < j) {
        swap(&buffer[i++], &buffer[j--]);
    }
    return buffer;
}

int abs(int v) {
    if (v < 0)
        return -v;
    return v;
}

// Iterative function to implement `itoa()` function in C
char *itoa(int value, char *buffer, int base) {
    // invalid input
    if (base < 2 || base > 32) {
        return buffer;
    }

    // consider the absolute value of the number
    int n = abs(value);

    int i = 0;
    while (n) {
        int r = n % base;

        if (r >= 10) {
            buffer[i++] = 65 + (r - 10);
        } else {
            buffer[i++] = 48 + r;
        }

        n = n / base;
    }

    // if the number is 0
    if (i == 0) {
        buffer[i++] = '0';
    }

    // If the base is 10 and the value is negative, the resulting string
    // is preceded with a minus sign (-)
    // With any other base, value is always considered unsigned
    if (value < 0 && base == 10) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0'; // null terminate string

    // reverse the string and return it
    return reverse(buffer, 0, i - 1);
}

static char *rand_string(int size) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890+/*-.!@#$%%^&*()_+<>?:\"{}|-=[]\\;',./";
    static char str[256];

    size %= 256;

    if (size) {
        --size;
        int n;
        for (n = 0; n < size; n++) {
            int key = rand() % (int)(sizeof charset - 1);
            str[n] = charset[key];
        }
        str[size] = '\0';
    }

    return str;
}

int lvl = 0;
int depth = 1;
int child = 0;

void test(int less) {
    char buf[256];
    buf[0] = '.';
    buf[1] = '/';
    int pos = 1;
    int i;
    while (lvl < less) {
        if (randr(0, 1024) < 256) {
            if (pos != 1)
                buf[++pos] = '/';
            strcpy(buf + pos + 1, "lvl");
            pos += strlen("lvl");
            itoa(++lvl, buf + pos + 1, 10);
            pos += strlen(buf + pos);
            --pos;
        }
        mkdir(buf);
        if (depth != 0 && randr(0, 1024) < 32 && fork() == 0) {
            child = 1;
            depth--;
            lvl = 0;
        } else {
            wait();
        }
        for (i = 0; i < 5; i++) {
            if (randr(0, 1024) < 64) {
                if (pos != 1)
                    buf[pos + 1] = '/';
                strcpy(buf + pos + 2, "file");
                itoa(i, buf + pos + 6, 10);

                int f = open(buf, O_CREATE | O_WRONLY);
                int j;
                for (j = 0; j < rand() % randr(1, 32); j++) {
                    printf(f, "%s", rand_string(randr(0, 256)));
                }
                close(f);
            }
        }
    }
}

char *fmtname(char *path) {
    static char buf[DIRSIZ + 1];
    char *p;

    // Find first character after last slash.
    for (p = path + strlen(path); p >= path && *p != '/'; p--)
        ;
    p++;

    // Return blank-padded name.
    if (strlen(p) >= DIRSIZ)
        return p;
    memmove(buf, p, strlen(p));
    memset(buf + strlen(p), '\00', DIRSIZ - strlen(p));
    return buf;
}

typedef struct dirChain {
    uint inum;
    uint size;
    short type;
    int nlen;
    char name[DIRSIZ];
    struct dirChain *prev;
    struct dirChain *next;
} dirChain;

int wither(char *path) {
    char buf[strlen(path) + 1 + DIRSIZ + 1], *p;
    int fd;
    struct dirent de;
    struct stat st;

    dirChain *chain = malloc(sizeof(dirChain));

    chain->next = chain;
    chain->prev = chain;

    dirChain *headChain = chain;
    dirChain *newChain;

    if ((fd = open(path, 0)) < 0) {
        printf(2, "test: cannot open %s\n", path);
        return -1;
    }

    if (fstat(fd, &st) < 0) {
        printf(2, "test: cannot stat %s\n", path);
        close(fd);
        return -1;
    }

    switch (st.type) {
    case T_FILE:
        printf(1, "Use on a directory to wither\n");
        break;

    case T_DIR:

        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf) {
            printf(1, "test: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;

            if (stat(buf, &st) < 0) {
                printf(1, "test: cannot stat %s\n", buf);
                continue;
            }

            switch (st.type) {
            case T_FILE:
                break;

            case T_DIR:
                if (strcmp(fmtname(buf), ".") && strcmp(fmtname(buf), "..")) { // Ignore system dirs
                    newChain = malloc(sizeof(dirChain));
                    newChain->type = st.type;
                    newChain->size = st.size;
                    newChain->inum = st.ino;
                    newChain->nlen = strlen(fmtname(buf));
                    memmove(newChain->name, fmtname(buf), DIRSIZ); // might overread

                    newChain->next = chain->next;
                    newChain->prev = chain;

                    chain->next->prev = newChain;
                    chain->next = newChain;

                    chain = chain->next;
                }
            }
        }
        break;
    }
    close(fd);

    chain = headChain->next; // Start from beginning
    while (chain != headChain) {
        memset(buf, 0, sizeof(buf));
        strcpy(buf, path);
        strcpy(buf + strlen(path), "/");
        memmove(buf + strlen(path) + 1, chain->name, chain->nlen);

        if (randr(0, 100) > 90 || ((headChain->next == headChain) && (randr(0, 100) < 50)))
            funlink(buf);
        else
            wither(buf);

        chain = chain->next;
        free(chain->prev);
    }
    free(headChain);

    return 0;
}

int main(int argc, char *argv[]) {
    int i;

    printf(1, "Creating Dummy Directory Tree\n");
    for (i = 0; i < 13; i += 3) {
        printf(1, "%d / %d\n", i, 12);
        test(i);
    }

    if (child || argc > 1) // don't continue if given an argument
        exit();

    printf(1, "Withering Current Directories\n");
    for (i = 0; i < 13; i += 3) {
        printf(1, "%d / %d\n", i, 12);
        wither(".");
    }

    if (argc > 2) // don't continue if given two arguments
        exit();

    printf(1, "Running recovery program\n");
    if (fork() == 0) {
        char *arg[1] = {"recoveryWalker"};
        exec(arg[0], arg);
        printf(2, "running recovery failed\n");
        exit();
    }
    wait();
    exit();
}

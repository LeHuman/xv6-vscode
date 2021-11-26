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

int main(int argc, char *argv[]) {
    int i;
    for (i = 0; i < 13; i += 3) {
        printf(1, "%d / %d\n", i, 12);
        test(i);
    }
    exit();
}

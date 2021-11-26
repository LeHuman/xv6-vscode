#include "types.h"
#include "stat.h"
#include "user.h"
#include "fs.h"
#include "fcntl.h"

unsigned long randstate = 1;
unsigned int rand() {
    randstate = randstate * 1664525 + 1013904223;
    return randstate % 1024;
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

int lvl = 0;

int depth = 1;

void test(int less) {
    char buf[256];
    buf[0] = '.';
    buf[1] = '/';
    int pos = 1;
    int i;
    while (lvl < less) {
        if (rand() < 256) {
            if (pos != 1)
                buf[++pos] = '/';
            strcpy(buf + pos + 1, "lvl");
            pos += strlen("lvl");
            itoa(++lvl, buf + pos + 1, 10);
            pos += strlen(buf + pos);
            --pos;
        }
        mkdir(buf);
        if (depth != 0 && rand() < 32 && fork() == 0) {
            depth--;
            lvl = 0;
            randstate = rand() % (rand() % 512);
        } else {
            wait();
        }
        for (i = 0; i < 5; i++) {
            if (rand() < 64) {
                if (pos != 1)
                    buf[pos + 1] = '/';
                strcpy(buf + pos + 2, "file");
                itoa(i, buf + pos + 6, 10);
                close(open(buf, O_CREATE));
            }
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc > 1)
        randstate = (long unsigned int)argv[1];
    test(4);
    test(8);
    test(11);
    test(13);
    exit();
}

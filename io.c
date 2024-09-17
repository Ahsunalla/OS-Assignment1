#include <errno.h>
#include <unistd.h>
#include "io.h"



/* Reads next char from stdin. If no more characters, it returns EOF */
int read_char() {
    char c;
    ssize_t result = read(0, &c, 1);  // Reading 1 character from stdin
    if (result == 1) {
        return c;  // Return the character read
    } else if (result == 0) {
        return EOF;  // End of file
    } else {
        return EOF;  // Error occurred
    }
}

int write_char(char c) {
    ssize_t result = write(1, &c, 1);  // Writing 1 character to stdout
    if (result == 1) {
        return 0;  // Success
    } else {
        return EOF;  
    }
}

/* Writes c to stdout.  If no errors occur, it returns 0, otherwise EOF */
int write_string(char* s) {
    while (*s != '\0') {
        if (write_char(*s) == EOF) {
            return EOF;  // If any write fails, return EOF
        }
        s++;
    }
    return 0; 
}


/* Writes a null-terminated string to stdout.  If no errors occur, it returns 0, otherwise EOF */
int write_int(int n) {
    char buffer[12];  // Enough to hold the string representation of a 32-bit int
    char* ptr = buffer + sizeof(buffer) - 1;
    int is_negative = 0;

    *ptr = '\0';  // null-terminate the string

    if (n == 0) {
        *--ptr = '0';
    } else {
        if (n < 0) {
            is_negative = 1;
            n = -n;
        }
        while (n > 0) {
            *--ptr = (n % 10) + '0';  // Extract digit
            n /= 10;
        }
        if (is_negative) {
            *--ptr = '-';  // minus sign for negative numbers
        }
    }

    return write_string(ptr);  // returns resulting string here
}


#include <stdio.h>

void run() {
    char *command = "/bin/sh";

    fwrite("Good... Wait what?\n", sizeof(char), 19, stdout);
    system(command);
}

void main() {
    char str[76];

    gets(str);
}
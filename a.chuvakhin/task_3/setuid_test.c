#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define DATA_FILE "data.txt"

static void print_ids(const char *label)
{
    printf("--- %s ---\n", label);
    printf("Real UID:      %d\n", (int)getuid());
    printf("Effective UID: %d\n", (int)geteuid());
}

static void try_open(const char *label)
{
    FILE *f = fopen(DATA_FILE, "r");
    if (f == NULL) {
        printf("%s: ", label);
        fflush(stdout);
        perror("fopen failed");
    } else {
        printf("%s: file '%s' opened successfully\n", label, DATA_FILE);
        fclose(f);
    }
}

int main(void)
{
    /* 1. Идентификаторы и попытка доступа ДО сброса привилегий */
    print_ids("Before setuid(getuid())");
    try_open("Access attempt #1");

    /* 2. Сброс привилегий: эффективный UID становится равным реальному */
    if (setuid(getuid()) != 0) {
        perror("setuid failed");
        exit(EXIT_FAILURE);
    }

    /* 3. Идентификаторы и попытка доступа ПОСЛЕ сброса привилегий */
    print_ids("After setuid(getuid())");
    try_open("Access attempt #2");

    return EXIT_SUCCESS;
}

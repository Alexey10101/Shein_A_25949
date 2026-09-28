#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void print_uid(void) {
    printf("Real UID: %d\n", (int)getuid());
    printf("Effective UID: %d\n", (int)geteuid());
}

void check_file(void) {
    FILE *file = fopen("data.txt", "r");
    if (file == NULL) {
        perror("Cannot open data.txt");

    }else {
        printf("data.txt opened successfully");
        fclose(file);
    }
}

int main(void) {
    printf("Before setuid():\n");
    print_uid();

    printf("Trying to open data.txt:\n");
    check_file();
    printf("\nCalling setuid(getuid())..."\n);
    if (setuid(getuid()) == -1) {
        perror("setuid");
        return EXIT_FAILURE;
    }

    printf("\nAfter setuid():\n");
    print_uid();
    printf("Trying to open data.txt again:\n");
    check_file();

    return EXIT_SUCCESS;
}

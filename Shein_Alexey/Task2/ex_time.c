#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void)
{
    time_t now;
    struct tm *sp;
    const char *zone;

    if (setenv("TZ", "America/Los_Angeles", 1) == -1) {
        perror("setenv");
        return EXIT_FAILURE;
    }

    tzset();

    now = time(NULL);
    if (now == (time_t)-1) {
        perror("time");
        return EXIT_FAILURE;
    }

    sp = localtime(&now);
    if (sp == NULL) {
        perror("localtime");
        return EXIT_FAILURE;
    }

    if (sp->tm_isdst < 0) {
        fprintf(stderr, "Cannot determine daylight saving time status\n");
        return EXIT_FAILURE;
    }

    zone = tzname[sp->tm_isdst > 0 ? 1 : 0];

    printf("%02d/%02d/%04d %02d:%02d %s\n",
           sp->tm_mon + 1,
           sp->tm_mday,
           sp->tm_year + 1900,
           sp->tm_hour,
           sp->tm_min,
           zone);

    return EXIT_SUCCESS;
}

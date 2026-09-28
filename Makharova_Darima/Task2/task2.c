#define _POSIX_C_SOURCE 200809L
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

extern char *tzname[];

int main(void)
{
    time_t now;
    struct tm *sp;

    if (setenv("TZ", "PST8", 1) == -1) {
        perror("setenv");
        return 1;
    }
    tzset();

    (void) time(&now);

    printf("%s", ctime(&now));

    sp = localtime(&now);
    printf("%d/%d/%04d %d:%02d %s\n",
        sp->tm_mon + 1, sp->tm_mday,
        sp->tm_year + 1900, sp->tm_hour,
        sp->tm_min, tzname[sp->tm_isdst]);

    return 0;
}
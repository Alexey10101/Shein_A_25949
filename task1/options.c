#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

extern char **environ;

/* one parsed option: letter + optional argument */
typedef struct {
    char opt;
    char *arg;
} opt_t;

int main(int argc, char *argv[]) {
    const char *optstring = "ispuU:cC:dvV:";
    opt_t opts[256];
    int n = 0;
    int c;

    /* parse all options with getopt */
    while ((c = getopt(argc, argv, optstring)) != -1) {
        if (c == '?') {
            fprintf(stderr, "invalid option: -%c\n", optopt);
            return 1;
        }
        opts[n].opt = (char)c;
        opts[n].arg = optarg ? strdup(optarg) : NULL; /* copy: optarg is reused */
        n++;
    }

    /* run options right to left (reverse order of appearance) */
    for (int i = n - 1; i >= 0; i--) {
        char o = opts[i].opt;
        char *a = opts[i].arg;

        switch (o) {
        case 'i': /* print real/effective uid and gid */
            printf("-i: uid=%d euid=%d gid=%d egid=%d\n",
                   getuid(), geteuid(), getgid(), getegid());
            break;

        case 's': /* become process group leader */
            if (setpgid(0, 0) == -1)
                perror("-s: setpgid");
            else
                printf("-s: pgid=%d\n", getpgrp());
            break;

        case 'p': /* print pid, ppid, pgid */
            printf("-p: pid=%d ppid=%d pgid=%d\n",
                   getpid(), getppid(), getpgrp());
            break;

        case 'u': { /* print ulimit (open files) */
            struct rlimit rl;
            if (getrlimit(RLIMIT_NOFILE, &rl) == 0)
                printf("-u: soft=%lu hard=%lu\n",
                       (unsigned long)rl.rlim_cur, (unsigned long)rl.rlim_max);
            else
                perror("-u: getrlimit");
            break;
        }

        case 'U': { /* change ulimit */
            errno = 0;
            char *end;
            long val = strtol(a, &end, 10);
            if (errno != 0 || *end != '\0' || val < 0) {
                fprintf(stderr, "-U: invalid value '%s'\n", a);
                break;
            }
            struct rlimit rl;
            if (getrlimit(RLIMIT_NOFILE, &rl) == -1) { perror("-U: getrlimit"); break; }
            rl.rlim_cur = (rlim_t)val;
            if (setrlimit(RLIMIT_NOFILE, &rl) == -1)
                perror("-U: setrlimit");
            else
                printf("-U: set to %ld\n", val);
            break;
        }

        case 'c': { /* print core file size */
            struct rlimit rl;
            if (getrlimit(RLIMIT_CORE, &rl) == 0)
                printf("-c: core=%lu\n", (unsigned long)rl.rlim_cur);
            else
                perror("-c: getrlimit");
            break;
        }

        case 'C': { /* change core file size */
            errno = 0;
            char *end;
            long val = strtol(a, &end, 10);
            if (errno != 0 || *end != '\0' || val < 0) {
                fprintf(stderr, "-C: invalid value '%s'\n", a);
                break;
            }
            struct rlimit rl;
            if (getrlimit(RLIMIT_CORE, &rl) == -1) { perror("-C: getrlimit"); break; }
            rl.rlim_cur = (rlim_t)val;
            if (setrlimit(RLIMIT_CORE, &rl) == -1)
                perror("-C: setrlimit");
            else
                printf("-C: set to %ld\n", val);
            break;
        }

        case 'd': { /* print current working directory */
            char buf[PATH_MAX];
            if (getcwd(buf, sizeof(buf)) != NULL)
                printf("-d: cwd=%s\n", buf);
            else
                perror("-d: getcwd");
            break;
        }

        case 'v': { /* print all environment variables */
            printf("-v: environment:\n");
            for (char **e = environ; *e; e++)
                printf("    %s\n", *e);
            break;
        }

        case 'V': { /* set env variable NAME=value */
            if (putenv(a) == 0)
                printf("-V: set %s\n", a);
            else
                perror("-V: putenv");
            break;
        }
        }
    }

    /* free duplicated arÐgs */
    for (int i = 0; i < n; i++) {
        if (opts[i].arg && opts[i].opt != 'V')
            free(opts[i].arg);
    }
    return 0;
}

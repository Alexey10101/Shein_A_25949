#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

extern char **environ;

typedef struct {
    char opt;
    char *arg;
} OptEntry;

int main(int argc, char *argv[]) {
    char *optstring = "ispuU:cC:dvV:";
    int c;
    OptEntry opts[256];
    int opt_count = 0;

    /* Сбор опций (getopt идёт слева направо) */
    while ((c = getopt(argc, argv, optstring)) != -1) {
        if (c == '?') {
            fprintf(stderr, "Invalid option: -%c\n", optopt);
            return 1;
        }
        opts[opt_count].opt = (char)c;
        opts[opt_count].arg = optarg ? strdup(optarg) : NULL;
        opt_count++;
    }

    /* Выполнение справа налево */
    for (int i = opt_count - 1; i >= 0; i--) {
        char opt = opts[i].opt;
        char *arg = opts[i].arg;

        switch (opt) {
            case 'i': {
                printf("-i: UID=%d, EUID=%d, GID=%d, EGID=%d\n",
                       getuid(), geteuid(), getgid(), getegid());
                break;
            }
            case 's': {
                if (setpgid(0, 0) == -1) {
                    perror("-s: setpgid failed");
                } else {
                    printf("-s: Process is now group leader. PGID=%d\n", getpgrp());
                }
                break;
            }
            case 'p': {
                printf("-p: PID=%d, PPID=%d, PGID=%d\n",
                       getpid(), getppid(), getpgrp());
                break;
            }
            case 'u': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
                    printf("-u: ulimit (NOFILE) soft=%lu, hard=%lu\n",
                           (unsigned long)rl.rlim_cur, (unsigned long)rl.rlim_max);
                } else {
                    perror("-u: getrlimit");
                }
                break;
            }
            case 'U': {
                struct rlimit rl;
                long new_val = atol(arg);
                if (new_val < 0) {
                    fprintf(stderr, "-U: invalid value '%s'\n", arg);
                    break;
                }
                if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
                    rl.rlim_cur = (rlim_t)new_val;
                    if (setrlimit(RLIMIT_NOFILE, &rl) == -1) {
                        perror("-U: setrlimit failed");
                    } else {
                        printf("-U: ulimit changed to %ld\n", new_val);
                    }
                }
                break;
            }
            case 'c': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                    printf("-c: core file size soft=%lu, hard=%lu\n",
                           (unsigned long)rl.rlim_cur, (unsigned long)rl.rlim_max);
                } else {
                    perror("-c: getrlimit");
                }
                break;
            }
            case 'C': {
                struct rlimit rl;
                long new_val = atol(arg);
                if (new_val < 0) {
                    fprintf(stderr, "-C: invalid value '%s'\n", arg);
                    break;
                }
                if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                    rl.rlim_cur = (rlim_t)new_val;
                    if (setrlimit(RLIMIT_CORE, &rl) == -1) {
                        perror("-C: setrlimit failed");
                    } else {
                        printf("-C: core size changed to %ld\n", new_val);
                    }
                }
                break;
            }
            case 'd': {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("-d: cwd=%s\n", cwd);
                } else {
                    perror("-d: getcwd");
                }
                break;
            }
            case 'v': {
                printf("-v: Environment variables:\n");
                for (char **env = environ; *env != NULL; env++) {
                    printf("    %s\n", *env);
                }
                break;
            }
            case 'V': {
                if (arg) {
                    if (putenv(arg) == 0) {
                        printf("-V: set %s\n", arg);
                    } else {
                        perror("-V: putenv failed");
                    }
                }
                break;
            }
        }
    }

    for (int i = 0; i < opt_count; i++) {
        if (opts[i].arg) free(opts[i].arg);
    }

    return 0;
}

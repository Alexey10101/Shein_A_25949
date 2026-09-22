
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/resource.h>

extern char **environ;

static const char *OPTSTRING = "ispuU:cC:dvV:";

typedef struct {
    char c;
    char *arg;
} OptEntry;

static void print_ids(void)
{
    printf("real uid=%ld  effective uid=%ld\n", (long)getuid(), (long)geteuid());
    printf("real gid=%ld  effective gid=%ld\n", (long)getgid(), (long)getegid());
}

static void become_pgrp_leader(void)
{
    if (setpgid(0, 0) == -1)
        fprintf(stderr, "-s: setpgid failed: %s\n", strerror(errno));
    else
        printf("process is now the leader of its process group (pgid=%ld)\n", (long)getpgrp());
}

static void print_pids(void)
{
    printf("pid=%ld  ppid=%ld  pgid=%ld\n",
           (long)getpid(), (long)getppid(), (long)getpgrp());
}

static void print_ulimit(void)
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
        fprintf(stderr, "-u: getrlimit failed: %s\n", strerror(errno));
        return;
    }
    if (rl.rlim_cur == RLIM_INFINITY)
        printf("ulimit (open files, soft) = unlimited\n");
    else
        printf("ulimit (open files, soft) = %ld\n", (long)rl.rlim_cur);
}

static void set_ulimit(const char *arg)
{
    char *end;
    errno = 0;
    long val = strtol(arg, &end, 10);
    if (end == arg || *end != '\0' || errno == ERANGE || val < 0) {
        fprintf(stderr, "-U: invalid ulimit value '%s'\n", arg);
        return;
    }

    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
        fprintf(stderr, "-U: getrlimit failed: %s\n", strerror(errno));
        return;
    }
    rl.rlim_cur = (rlim_t)val;
    if (rl.rlim_max != RLIM_INFINITY && rl.rlim_cur > rl.rlim_max)
        rl.rlim_max = rl.rlim_cur;
    if (setrlimit(RLIMIT_NOFILE, &rl) == -1)
        fprintf(stderr, "-U: setrlimit failed: %s\n", strerror(errno));
    else
        printf("ulimit (open files, soft) set to %ld\n", val);
}

static void print_core(void)
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
        fprintf(stderr, "-c: getrlimit failed: %s\n", strerror(errno));
        return;
    }
    if (rl.rlim_cur == RLIM_INFINITY)
        printf("core file size (soft) = unlimited\n");
    else
        printf("core file size (soft) = %ld bytes\n", (long)rl.rlim_cur);
}

static void set_core(const char *arg)
{
    char *end;
    errno = 0;
    long val = strtol(arg, &end, 10);
    if (end == arg || *end != '\0' || errno == ERANGE || val < 0) {
        fprintf(stderr, "-C: invalid core size value '%s'\n", arg);
        return;
    }

    struct rlimit rl;
    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
        fprintf(stderr, "-C: getrlimit failed: %s\n", strerror(errno));
        return;
    }
    rl.rlim_cur = (rlim_t)val;
    if (rl.rlim_max != RLIM_INFINITY && rl.rlim_cur > rl.rlim_max)
        rl.rlim_max = rl.rlim_cur;
    if (setrlimit(RLIMIT_CORE, &rl) == -1)
        fprintf(stderr, "-C: setrlimit failed: %s\n", strerror(errno));
    else
        printf("core file size (soft) set to %ld bytes\n", val);
}

static void print_cwd(void)
{
    char buf[PATH_MAX];
    if (getcwd(buf, sizeof(buf)) == NULL)
        fprintf(stderr, "-d: getcwd failed: %s\n", strerror(errno));
    else
        printf("cwd=%s\n", buf);
}

static void print_env(void)
{
    for (char **e = environ; *e != NULL; e++)
        printf("%s\n", *e);
}

static void set_env(const char *arg)
{
    if (strchr(arg, '=') == NULL) {
        fprintf(stderr, "-V: invalid argument '%s', expected name=value\n", arg);
        return;
    }
    char *copy = strdup(arg);
    if (copy == NULL) {
        fprintf(stderr, "-V: out of memory\n");
        return;
    }
    if (putenv(copy) != 0)
        fprintf(stderr, "-V: putenv failed: %s\n", strerror(errno));
    else
        printf("environment updated: %s\n", copy);
}

static void run_option(const OptEntry *e)
{
    switch (e->c) {
    case 'i': print_ids(); break;
    case 's': become_pgrp_leader(); break;
    case 'p': print_pids(); break;
    case 'u': print_ulimit(); break;
    case 'U': set_ulimit(e->arg); break;
    case 'c': print_core(); break;
    case 'C': set_core(e->arg); break;
    case 'd': print_cwd(); break;
    case 'v': print_env(); break;
    case 'V': set_env(e->arg); break;
    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    OptEntry *entries = NULL;
    size_t n = 0, cap = 0;
    int c;

    opterr = 0;
    while ((c = getopt(argc, argv, OPTSTRING)) != -1) {
        if (c == '?') {
            if (optopt == 'U' || optopt == 'C' || optopt == 'V')
                fprintf(stderr, "option -%c requires an argument\n", optopt);
            else
                fprintf(stderr, "invalid option: -%c\n", optopt);
            continue;
        }

        if (n == cap) {
            cap = cap ? cap * 2 : 8;
            OptEntry *tmp = realloc(entries, cap * sizeof(*entries));
            if (tmp == NULL) {
                fprintf(stderr, "out of memory\n");
                free(entries);
                return EXIT_FAILURE;
            }
            entries = tmp;
        }
        entries[n].c = (char)c;
        entries[n].arg = optarg;
        n++;
    }
    for (size_t i = n; i > 0; i--)
        run_option(&entries[i - 1]);

    free(entries);
    return EXIT_SUCCESS;
}

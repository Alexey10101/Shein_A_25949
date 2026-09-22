
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>       // PATH_MAX
#include <string.h>       // strdup, strchr
#include <errno.h>        // errno
#include <sys/resource.h>
extern char **environ;    // Глобальный указатель на массив переменных среды

#define MAX_OPTS 100

struct opt_entry {
    int opt;
    char *arg;
};

struct opt_entry opts[MAX_OPTS];
int n_opts = 0;

void do_i(void)
{
    printf("uid=%d, euid=%d, gid=%d, egid=%d\n", (int)getuid(), (int)geteuid(), (int)getgid(), (int)getegid());
}

/*uid (user id) - реальный, euid - от чьего имени процесс сецчас работает, 
gid (group id) - реальный, egid - группа, от имени которой сейчас процесс действует */

void do_s(void)
{
    if (setpgid(0, 0) == -1) //группа процессов, управление ими, первое - процесс, который переносим, второе - создаем новую группу с id=pid процесса
    {
        perror("setpgid");
    } else {
        printf("%d\n", (int)getpgrp());
    }
}

void do_p(void)
{
    printf("pid = %d, ppid = %d, pgid = %d\n", (int)getpid(), (int)getppid(), (int)getpgrp());
}

void do_d(void) //текущая рабочая директория
{
    char buf[PATH_MAX]; //определен в limits
    
    if (getcwd(buf, sizeof(buf)) == NULL)
    {
        perror("getcwd");
    } else
    {
        printf("cwd=%s\n", buf);
    }
}


void do_u(void) //ограничения на ресурсы, лимит количества открытых файлов
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == -1)
    {
        perror("getrlimit");
    } else
    {
        printf("current limit = %ld, max limit = %ld\n", (long)rl.rlim_cur, (long)rl.rlim_max);
    }
}

void do_U(char *arg) //изменяет лимит открытых файлов на значение, которое передано в аргументе опции
{
    struct rlimit rl;
    long val = atol(arg); //строку в число
    if (val < 0)
    {
        fprintf(stderr, "Invalid value: %s\n", arg);
        return;
    }
    

    if (getrlimit(RLIMIT_NOFILE, &rl) == -1) //узнаем rlim_max
    {
        perror("getrlimit");
        return;
    }

    rl.rlim_cur = (rlim_t)val; //меняем soft лимит

    if (setrlimit(RLIMIT_NOFILE, &rl) == -1) //записываем обратно
    {
        perror("setrlimit");
    } else
    {
        printf("ulimit set to %ld\n", val); 
    }
}

void do_c(void) //лимит размера core файла, который может быть создан
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_CORE, &rl) == -1)
    {
        perror("getrlimit");
    } else
    {
        printf("core size: soft = %ld, hard = %ld\n", (long)rl.rlim_cur, (long)rl.rlim_max);
    }
}

void do_C(char *arg) //изменяем размер core-файла
{
    struct rlimit rl;
    long val = atol(arg); //строку в число
    if (val < 0)
    {
        fprintf(stderr, "Invalid value: %s\n", arg);
        return;
    }
    

    if (getrlimit(RLIMIT_CORE, &rl) == -1) //узнаем rlim_max
    {
        perror("getrlimit");
        return;
    }

    rl.rlim_cur = (rlim_t)val; //меняем soft лимит

    if (setrlimit(RLIMIT_CORE, &rl) == -1) //записываем обратно
    {
        perror("setrlimit");
    } else
    {
        printf("core size set to %ld\n", val); 
    }
}

void do_v(void) //печатает все переменные окружения
{
    for (char**e = environ; *e != NULL; e++) //текущий элемент массива 
    {
        printf("%s\n", *e);
    }
}

void do_V(char *arg) //вносит новую переменную или изменяет значение сущ переменной
{
    if (strchr(arg, '=') == NULL)
    {
        fprintf(stderr, "-V: expected NAME=value\n");
        return;
    }
    if (putenv(arg) != 0)
    {
        perror("putenv");
    } else
    {
        printf("set: %s\n", arg);
    }
}


int main(int argc, char *argv[]) {
    // All options: s, p, u, d don't need arguments
    char *options = "ispuU:cC:dvV:";
    int c;
    
    while ((c = getopt(argc, argv, options)) != -1) { //разбирает аргументы и отделяет по одному
        if (c == '?') {
            fprintf(stderr, "Invalid option\n");
            exit(1);
        }
        opts[n_opts].opt = c;
        opts[n_opts].arg = optarg;
        n_opts++;
    }

    for (int i = n_opts - 1; i >= 0; i--) {
        switch (opts[i].opt) {
            case 'i': do_i(); break;
            case 's': do_s(); break;
            case 'p': do_p(); break;
            case 'u': do_u(); break;
            case 'U': do_U(opts[i].arg); break;
            case 'c': do_c(); break;
            case 'C': do_C(opts[i].arg); break;
            case 'd': do_d(); break;
            case 'v': do_v(); break;
            case 'V': do_V(opts[i].arg); break;
        }
    }
    
    return 0;
}


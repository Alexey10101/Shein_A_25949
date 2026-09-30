#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <limits.h>
#include <errno.h>

extern char **environ;

// Структура для сохранения опции и её аргумента
typedef struct
{
    int opt;
    char *arg;
} OptionItem;

// Функция выполнения конкретной опции
void execute_option(int opt, const char *arg)
{
    switch (opt)
    {
    case 'i':
    {
        printf("[ -i ] Real UID: %d, Effective UID: %d\n", getuid(), geteuid());
        printf("       Real GID: %d, Effective GID: %d\n", getgid(), getegid());
        break;
    }
    case 's':
    {
        if (setpgid(0, 0) == -1)
        {
            fprintf(stderr, "[ -s ] setpgid failed: %s\n", strerror(errno));
        }
        else
        {
            printf("[ -s ] Process became group leader (PGID: %d)\n", getpgrp());
        }
        break;
    }
    case 'p':
    {
        printf("[ -p ] PID: %d, PPID: %d, PGID: %d\n", getpid(), getppid(), getpgrp());
        break;
    }
    case 'u':
    {
        struct rlimit rlim;
        if (getrlimit(RLIMIT_FSIZE, &rlim) == -1)
        {
            fprintf(stderr, "[ -u ] getrlimit RLIMIT_FSIZE failed: %s\n", strerror(errno));
        }
        else
        {
            if (rlim.rlim_cur == RLIM_INFINITY)
            {
                printf("[ -u ] Current ulimit (file size): unlimited\n");
            }
            else
            {
                printf("[ -u ] Current ulimit (file size): %ld bytes\n", (long)rlim.rlim_cur);
            }
        }
        break;
    }
    case 'U':
    {
        if (!arg || *arg == '\0')
        {
            fprintf(stderr, "[ -U ] Missing argument\n");
            break;
        }
        char *endptr = NULL;
        errno = 0;
        long new_ulimit = strtol(arg, &endptr, 10);
        if (errno != 0 || *endptr != '\0' || new_ulimit < 0)
        {
            fprintf(stderr, "[ -U ] Invalid ulimit value: '%s'\n", arg);
        }
        else
        {
            struct rlimit rlim;
            if (getrlimit(RLIMIT_FSIZE, &rlim) == -1)
            {
                fprintf(stderr, "[ -U ] getrlimit RLIMIT_FSIZE failed: %s\n", strerror(errno));
            }
            else
            {
                rlim.rlim_cur = (rlim_t)new_ulimit;
                if (setrlimit(RLIMIT_FSIZE, &rlim) == -1)
                {
                    fprintf(stderr, "[ -U ] setrlimit RLIMIT_FSIZE failed: %s\n", strerror(errno));
                }
                else
                {
                    printf("[ -U ] ulimit set to %ld bytes\n", new_ulimit);
                }
            }
        }
        break;
    }
    case 'c':
    {
        struct rlimit rlim;
        if (getrlimit(RLIMIT_CORE, &rlim) == -1)
        {
            fprintf(stderr, "[ -c ] getrlimit RLIMIT_CORE failed: %s\n", strerror(errno));
        }
        else
        {
            if (rlim.rlim_cur == RLIM_INFINITY)
            {
                printf("[ -c ] Core file size limit: unlimited\n");
            }
            else
            {
                printf("[ -c ] Core file size limit: %ld bytes\n", (long)rlim.rlim_cur);
            }
        }
        break;
    }
    case 'C':
    {
        if (!arg || *arg == '\0')
        {
            fprintf(stderr, "[ -C ] Missing argument\n");
            break;
        }
        char *endptr = NULL;
        errno = 0;
        long new_core = strtol(arg, &endptr, 10);
        if (errno != 0 || *endptr != '\0' || new_core < 0)
        {
            fprintf(stderr, "[ -C ] Invalid core file size: '%s'\n", arg);
        }
        else
        {
            struct rlimit rlim;
            if (getrlimit(RLIMIT_CORE, &rlim) == -1)
            {
                fprintf(stderr, "[ -C ] getrlimit RLIMIT_CORE failed: %s\n", strerror(errno));
            }
            else
            {
                rlim.rlim_cur = (rlim_t)new_core;
                if (setrlimit(RLIMIT_CORE, &rlim) == -1)
                {
                    fprintf(stderr, "[ -C ] setrlimit RLIMIT_CORE failed: %s\n", strerror(errno));
                }
                else
                {
                    printf("[ -C ] Core file size set to %ld bytes\n", new_core);
                }
            }
        }
        break;
    }
    case 'd':
    {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("[ -d ] Current directory: %s\n", cwd);
        }
        else
        {
            fprintf(stderr, "[ -d ] getcwd failed: %s\n", strerror(errno));
        }
        break;
    }
    case 'v':
    {
        printf("[ -v ] Environment variables:\n");
        for (char **env = environ; *env != NULL; ++env)
        {
            printf("  %s\n", *env);
        }
        break;
    }
    case 'V':
    {
        if (!arg || strchr(arg, '=') == NULL)
        {
            fprintf(stderr, "[ -V ] Argument must be in format 'NAME=VALUE': '%s'\n", arg ? arg : "");
        }
        else
        {
            // putenv требует сохранения строки в памяти на протяжении работы процесса
            char *env_str = strdup(arg);
            if (!env_str || putenv(env_str) != 0)
            {
                fprintf(stderr, "[ -V ] putenv failed: %s\n", strerror(errno));
            }
            else
            {
                printf("[ -V ] Environment variable set: %s\n", arg);
            }
        }
        break;
    }
    case '?':
    {
        fprintf(stderr, "Invalid option or missing argument: -%c\n", (char)optopt);
        break;
    }
    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    const char *options = "ispduU:cC:vV:";
    int c;

    // Выделяем память под массив опций (максимально может быть не более argc элементов)
    OptionItem *parsed_options = (OptionItem *)malloc(argc * sizeof(OptionItem));
    if (!parsed_options)
    {
        perror("malloc failed");
        return 1;
    }
    int count = 0;

    // 1. Считываем опции слева направо с помощью getopt
    while ((c = getopt(argc, argv, options)) != -1)
    {
        parsed_options[count].opt = c;
        if (optarg != NULL)
        {
            parsed_options[count].arg = strdup(optarg);
        }
        else
        {
            parsed_options[count].arg = NULL;
        }
        count++;
    }

    if (count == 0)
    {
        printf("No options provided.\n");
    }

    // 2. Выполняем опции СПРАВА НАЛЕВО (в обратном порядке)
    for (int i = count - 1; i >= 0; i--)
    {
        execute_option(parsed_options[i].opt, parsed_options[i].arg);
    }

    // Освобождение выделенной памяти
    for (int i = 0; i < count; i++)
    {
        if (parsed_options[i].arg != NULL)
        {
            free(parsed_options[i].arg);
        }
    }
    free(parsed_options);

    // Вывод позиционных (не-опционных) аргументов
    if (optind < argc)
    {
        printf("Non-option arguments: ");
        for (int i = optind; i < argc; ++i)
        {
            printf("%s ", argv[i]);
        }
        printf("\n");
    }

    return 0;
}

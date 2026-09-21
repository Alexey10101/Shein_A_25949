#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/resource.h> // ограничения ресурсов процесса
#include <limits.h>
#include <errno.h> // для проверки ошибок преобразования строки в число
#include <string.h>

#define MAX_OPTIONS 100

extern char **environ; // массив переменных окружения процесса

typedef struct {
    char option;
    char *value;
} Option;

int main(int argc, char *argv[]) {
    Option options[MAX_OPTIONS]; // массив опций
    int option_count = 0; // количество опций
    int c; // возвращает найденную опцию

    opterr = 0;

    while ((c = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {

        if (option_count >= MAX_OPTIONS) {
            fprintf(stderr, "Too many options\n");
            return 1;
        }

        switch(c) {

            // группа обычных опций
            case 'i':
            case 's':
            case 'p':
            case 'u':
            case 'c':
            case 'd':
            case 'v':
                options[option_count].option = c;
                options[option_count].value = NULL;
                option_count++;
                break;

            // опции с аргументом
            case 'U':
            case 'C':
            case 'V':
                options[option_count].option = c;
                options[option_count].value = optarg;
                option_count++;
                break;

            case '?': // если ошибка
                if (optopt == 'U' || optopt == 'C' || optopt == 'V') {
                    fprintf(stderr, "Option -%c requires an argument\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option: -%c\n", optopt);
                }
                return 1;
        }
    }

    // выполняется справа налево
    for (int i = option_count - 1; i >= 0; i--) {

        switch(options[i].option) {

            /*
             * -i
             * Реальные и эффективные UID/GID.
             */
            case 'i':
                printf("UID: real=%d effective=%d\n",
                       (int)getuid(), (int)geteuid());

                printf("GID: real=%d effective=%d\n",
                       (int)getgid(), (int)getegid());
                break;

            /*
             * -s
             * Процесс становится лидером своей группы.
             */
            case 's':
                if (setpgid(0, 0) == -1) {
                    perror("setpgid");
                    return 1;
                }

                printf("Process group leader: PGID=%d\n",
                       (int)getpgrp());
                break;

            /*
             * -p
             * PID, PPID, PGID.
             */
            case 'p':
                printf("PID: %d\n", (int)getpid());
                printf("PPID: %d\n", (int)getppid());
                printf("PGID: %d\n", (int)getpgrp());
                break;

            /*
             * -u
             * Текущий ulimit.
             *
             * Здесь используем RLIMIT_NOFILE —
             * максимальное количество открытых файлов.
             */
            case 'u':
            {
                struct rlimit limit;

                if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
                    perror("getrlimit");
                    return 1;
                }

                printf("ulimit: %llu\n",
                       (unsigned long long)limit.rlim_cur);
                break;
            }

            /*
             * -Unew_ulimit
             * Изменение ulimit.
             */
            case 'U':
            {
                char *end;
                long value;

                errno = 0;
                end = NULL;

                value = strtol(options[i].value, &end, 10);

                if (errno != 0 ||
                    end == options[i].value ||
                    *end != '\0' ||
                    value < 0) {

                    fprintf(stderr,
                            "Invalid value for -U: %s\n",
                            options[i].value);
                    return 1;
                }

                struct rlimit limit;

                if (getrlimit(RLIMIT_NOFILE, &limit) == -1) {
                    perror("getrlimit");
                    return 1;
                }

                limit.rlim_cur = (rlim_t)value;

                if (setrlimit(RLIMIT_NOFILE, &limit) == -1) {
                    perror("setrlimit");
                    return 1;
                }

                printf("ulimit changed to %ld\n", value);
                break;
            }

            /*
             * -c
             * Размер core-файла.
             */
            case 'c':
            {
                struct rlimit limit;

                if (getrlimit(RLIMIT_CORE, &limit) == -1) {
                    perror("getrlimit");
                    return 1;
                }

                printf("core size: %llu bytes\n",
                       (unsigned long long)limit.rlim_cur);
                break;
            }

            /*
             * -Csize
             * Изменение размера core-файла.
             */
            case 'C':
            {
                char *end;
                long value;

                errno = 0;
                end = NULL;

                value = strtol(options[i].value, &end, 10);

                if (errno != 0 ||
                    end == options[i].value ||
                    *end != '\0' ||
                    value < 0) {

                    fprintf(stderr,
                            "Invalid value for -C: %s\n",
                            options[i].value);
                    return 1;
                }

                struct rlimit limit;

                if (getrlimit(RLIMIT_CORE, &limit) == -1) {
                    perror("getrlimit");
                    return 1;
                }

                limit.rlim_cur = (rlim_t)value;

                if (setrlimit(RLIMIT_CORE, &limit) == -1) {
                    perror("setrlimit");
                    return 1;
                }

                printf("core size changed to %ld bytes\n", value);
                break;
            }

            /*
             * -d
             * Текущая директория.
             */
            case 'd':
            {
                char cwd[PATH_MAX];

                if (getcwd(cwd, sizeof(cwd)) == NULL) {
                    perror("getcwd");
                    return 1;
                }

                printf("Current directory: %s\n", cwd);
                break;
            }

            /*
             * -v
             * Все переменные окружения.
             */
            case 'v':
            {
                for (char **env = environ; *env != NULL; env++) {
                    printf("%s\n", *env);
                }

                break;
            }

            /*
             * -Vname=value
             * Добавление/изменение переменной.
             */
            case 'V':
                if (putenv(options[i].value) != 0) {
                    perror("putenv");
                    return 1;
                }

                printf("Environment changed: %s\n",
                       options[i].value);
                break;
        }
    }

    return 0;
}

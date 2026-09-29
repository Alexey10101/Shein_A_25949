#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <limits.h>
#include <string.h>

extern char **environ;

typedef struct {
    char opt;
    char *arg;
} OptionRecord;

#define MAX_OPTS 100
OptionRecord records[MAX_OPTS];
int opt_count = 0;

int main(int argc, char *argv[]) {
    int c;
    
    while ((c = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        if (c == '?') {
            continue;
        }
        if (opt_count < MAX_OPTS) {
            records[opt_count].opt = c;
            records[opt_count].arg = optarg ? strdup(optarg) : NULL;
            opt_count++;
        }
    }
    
    for (int i = opt_count - 1; i >= 0; i--) {
        char current_opt = records[i].opt;
        char *current_arg = records[i].arg;
        
        switch (current_opt) {
            case 'i':
                printf("[-i] UID: %d, EUID: %d, GID: %d, EGID: %d\n", 
                       getuid(), geteuid(), getgid(), getegid());
                break;
                
            case 's':
                if (setpgid(0, 0) == 0) {
                    printf("[-s] Процесс стал лидером группы.\n");
                } else {
                    perror("[-s] Ошибка setpgid");
                }
                break;
                
            case 'p':
                printf("[-p] PID: %d, PPID: %d, PGID: %d\n", 
                       getpid(), getppid(), getpgid(0));
                break;
                
            case 'u': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
                    printf("[-u] Ulimit (NOFILE): %ld\n", (long)rl.rlim_cur);
                } else {
                    perror("[-u] Ошибка getrlimit");
                }
                break;
            }
                
            case 'U': {
                long new_limit = atol(current_arg);
                if (new_limit <= 0) {
                    fprintf(stderr, "[-U] Ошибка: Неудачное значение для U: %s\n", current_arg);
                } else {
                    struct rlimit rl;
                    getrlimit(RLIMIT_NOFILE, &rl);
                    rl.rlim_cur = new_limit;
                    if (setrlimit(RLIMIT_NOFILE, &rl) == 0) {
                        printf("[-U] Значение Ulimit (NOFILE) изменено на %ld\n", new_limit);
                    } else {
                        perror("[-U] Ошибка setrlimit");
                    }
                }
                break;
            }
                
            case 'c': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                    printf("[-c] Core file size limit: %ld\n", (long)rl.rlim_cur);
                } else {
                    perror("[-c] Ошибка getrlimit");
                }
                break;
            }
                
            case 'C': {
                long new_size = atol(current_arg);
                if (new_size < 0) {
                    fprintf(stderr, "[-C] Ошибка: Неудачное значение для C: %s\n", current_arg);
                } else {
                    struct rlimit rl;
                    getrlimit(RLIMIT_CORE, &rl);
                    rl.rlim_cur = new_size;
                    if (setrlimit(RLIMIT_CORE, &rl) == 0) {
                        printf("[-C] Значение Core file size изменено на %ld\n", new_size);
                    } else {
                        perror("[-C] Ошибка setrlimit");
                    }
                }
                break;
            }
                
            case 'd': {
                char cwd[PATH_MAX];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("[-d] Текущая директория: %s\n", cwd);
                } else {
                    perror("[-d] Ошибка getcwd");
                }
                break;
            }
                
            case 'v':
                printf("[-v] Переменные среды:\n");
                for (char **env = environ; *env != 0; env++) {
                    printf("  %s\n", *env);
                }
                break;
                
            case 'V':
                if (putenv(current_arg) == 0) {
                    printf("[-V] Переменная среды добавлена/изменена: %s\n", current_arg);
                } else {
                    perror("[-V] Ошибка putenv");
                }
                break;
        }
    }
    
    return 0;
}

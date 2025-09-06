#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pwd.h>
#include <libgen.h>
#include <errno.h>

#define MAX_INPUT 512
#define MAX_ARGS 32

// Функция для отображения приглашения
void print_prompt() {
    char cwd[1024];
    char hostname[256];
    char *user;
    struct passwd *pw;
    
    // Получаем текущего пользователя
    pw = getpwuid(getuid());
    user = pw->pw_name;
    
    // Получаем имя хоста
    gethostname(hostname, sizeof(hostname));
    
    // Получаем текущий каталог
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        // Преобразуем путь в DOS-стиль (C:\>)
        if (strncmp(cwd, "/mnt/c", 6) == 0) {
            printf("C:%s> ", cwd + 6);
        } else {
            // Показываем только базовое имя каталога
            char *base = basename(cwd);
            printf("%s@%s:%s> ", user, hostname, base);
        }
    } else {
        printf("%s@%s:$ ", user, hostname);
    }
    fflush(stdout);
}

// Функция для разбивки ввода на аргументы
int parse_input(char *input, char *args[]) {
    int i = 0;
    char *token = strtok(input, " \t\n");
    
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
    
    return i;
}

// Функция для выполнения встроенных команд DOS
int execute_builtin(char *args[]) {
    if (args[0] == NULL) {
        return 1;
    }
    
    // Команда cd/chdir
    if (strcmp(args[0], "cd") == 0 || strcmp(args[0], "chdir") == 0) {
        if (args[1] == NULL) {
            chdir(getenv("HOME"));
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
        return 1;
    }
    
    // Команда dir
    if (strcmp(args[0], "dir") == 0) {
        char *ls_args[MAX_ARGS] = {"ls", "-l", "--color=auto", NULL};
        int i = 1, j = 1;
        
        // Обработка параметров DOS
        while (args[i] != NULL) {
            if (strcmp(args[i], "/w") == 0) {
                ls_args[1] = "-C";  // Широкий формат
            } else if (strcmp(args[i], "/p") == 0) {
                ls_args[j++] = "--more";
            } else {
                ls_args[j++] = args[i];
            }
            i++;
        }
        ls_args[j] = NULL;
        
        // Выполняем команду ls
        pid_t pid = fork();
        if (pid == 0) {
            execvp("ls", ls_args);
            perror("ls");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            wait(NULL);
        } else {
            perror("fork");
        }
        return 1;
    }
    
    // Команда cls/clear
    if (strcmp(args[0], "cls") == 0 || strcmp(args[0], "clear") == 0) {
        system("clear");
        return 1;
    }
    
    // Команда copy
    if (strcmp(args[0], "copy") == 0) {
        if (args[1] == NULL || args[2] == NULL) {
            printf("Usage: copy <source> <destination>\n");
            return 1;
        }
        
        char *cp_args[] = {"cp", "-r", args[1], args[2], NULL};
        pid_t pid = fork();
        if (pid == 0) {
            execvp("cp", cp_args);
            perror("cp");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            wait(NULL);
        } else {
            perror("fork");
        }
        return 1;
    }
    
    // Команда del/erase
    if (strcmp(args[0], "del") == 0 || strcmp(args[0], "erase") == 0) {
        if (args[1] == NULL) {
            printf("Usage: del <file>\n");
            return 1;
        }
        
        char *rm_args[] = {"rm", args[1], NULL};
        pid_t pid = fork();
        if (pid == 0) {
            execvp("rm", rm_args);
            perror("rm");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            wait(NULL);
        } else {
            perror("fork");
        }
        return 1;
    }
    
    // Команда ver
    if (strcmp(args[0], "ver") == 0) {
        system("uname -a");
        printf("GnidOS v0.1 - DOS-like Linux distribution\n");
        return 1;
    }
    
    // Команда help
    if (strcmp(args[0], "help") == 0 || strcmp(args[0], "/?") == 0) {
        system("cat /usr/share/gnidos/help.txt");
        return 1;
    }
    
    // Команда exit
    if (strcmp(args[0], "exit") == 0) {
        exit(EXIT_SUCCESS);
    }
    
    return 0;  // Не встроенная команда
}

int main() {
    char input[MAX_INPUT];
    char *args[MAX_ARGS];
    
    printf("GnidOS DOS Emulator v0.1\n");
    printf("Type \"help\" or \"/?\" for available commands\n\n");
    
    while (1) {
        print_prompt();
        
        if (!fgets(input, MAX_INPUT, stdin)) {
            break;
        }
        
        // Пропускаем пустые строки
        if (strspn(input, " \t\n") == strlen(input)) {
            continue;
        }
        
        // Разбираем ввод на аргументы
        int argc = parse_input(input, args);
        if (argc == 0) {
            continue;
        }
        
        // Пытаемся выполнить как встроенную команду
        if (execute_builtin(args)) {
            continue;
        }
        
        // Выполнение внешней команды
        pid_t pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            
            // Если команда не найдена, пробуем найти в /usr/bin
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "/usr/bin/%s", args[0]);
            execv(full_path, args);
            
            perror("Command not found");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            wait(NULL);
        } else {
            perror("fork");
        }
    }
    
    return 0;
}


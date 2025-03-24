#include <ctype.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>

typedef enum error {
    OK,
    INVALID_INPUT,
    MEMORY_ERROR,
    PROBLEMS_WITH_FILE,
    PROBLEMS_WITH_PID,
    NOT_A_NUMBER,
} error;

error checkN(const char *N);

error сopyN(const char *FileName, const int *NeedCopies);

error find(char const *FindString, char **FindFiles, const char *Files[], const int *CountFiles, int *size, int *capacity);

int main(int argc, char const *argv[]) {
    int i, N, size = 0, capacity = 2;
    char *AnswerString;

    if (argc < 3) {
        printf("Invalid input\n");
        return INVALID_INPUT;
    }

    for (i = 0; i < argc; i++) {
        if (argv[i] == NULL) {
            printf("Invalid input\n");
            return INVALID_INPUT;
        }
    }

    if (strncmp(argv[argc - 1], "copy", 4) == 0) {
        if (checkN(argv[argc - 1]) != OK) {
            printf("Error with a number\n");
        } else {
            N = strtol(argv[argc - 1] + 4, NULL, 10);
            if (N < 0 || N > 10) {
                printf("Too many copies\n");
                return INVALID_INPUT;
            }

            for (i = 1; i < argc - 1; ++i) {
                switch (сopyN(argv[i], &N)) {
                    case MEMORY_ERROR:
                        printf("Memory error. %s not processed\n", argv[i]);
                    break;
                    case PROBLEMS_WITH_FILE:
                        printf("Problem with file. %s not processed\n", argv[i]);
                    break;
                    case PROBLEMS_WITH_PID:
                        printf("Problem with pid. %s not processed\n", argv[i]);
                    break;
                    default:
                        printf("File %s successful processed\n", argv[i]);
                    break;
                }
            }
        }
    } else if (strncmp(argv[argc - 2], "find", 4) == 0) {
        AnswerString = (char *) malloc(capacity * sizeof(char));
        N = argc - 3;

        switch (find(argv[argc - 1], &AnswerString, &argv[0], &N, &size, &capacity)) {
            case MEMORY_ERROR:
                printf("Memory error\n");
                free(AnswerString);
                break;
            case PROBLEMS_WITH_FILE:
                printf("Problem with file\n");
                free(AnswerString);
                break;
            case PROBLEMS_WITH_PID:
                printf("Problem with pid\n");
                free(AnswerString);
                break;
            default:
                printf("Files successful processed\n");

                for (i = 0; i < size; ++i) {
                    printf("Path: %s\n", &AnswerString[i]);
                }

                break;
        }

        free(AnswerString);
    }

    return 0;
}

error сopyN(const char *FileName, const int *NeedCopies) {
    if (!FileName || !NeedCopies) {
        return MEMORY_ERROR;
    }

    int len, i, symbol, status, has_error = OK;
    pid_t pid;
    char expansion[10]; // Расширение
    char name[256]; // Имя без точки и расширения
    char NewFileName[270];
    char *SearchDot;
    FILE *file = fopen(FileName, "r"), *NewFile;
    if (!file) {
        return PROBLEMS_WITH_FILE;
    }

    SearchDot = strrchr(FileName, '.');
    if (!SearchDot) {
        fclose(file);
        return PROBLEMS_WITH_FILE;
    }

    strcpy(expansion, SearchDot);
    expansion[strlen(expansion)] = '\0';
    len = SearchDot - FileName;
    strncpy(name, FileName, len);
    name[len] = '\0';

    for (i = 0; i < *NeedCopies; ++i) {
        pid = fork();

        if (pid < 0) {
            exit(PROBLEMS_WITH_PID);
        }

        if (pid == 0) {

            file = fopen(FileName, "r");
            if (!file) {
                exit(PROBLEMS_WITH_FILE);
            }

            snprintf(NewFileName, sizeof(NewFileName), "%s (%d)%s", name, i + 1, expansion);
            NewFile = fopen(NewFileName, "w");

            if (!NewFile) {
                fclose(file);
                exit(PROBLEMS_WITH_FILE);
            }

            while ((symbol = fgetc(file)) != EOF) {
                fputc(symbol, NewFile);
            }

            fclose(NewFile);
            fclose(file);
            exit(OK);
        }
    }

    for (i = 0; i < *NeedCopies; i++) {
        wait(&status);
        // WIFEXITED(status) проверяет, завершился ли процесс нормально через exit()
        // WEXITSTATUS(status) получает код возврата дочернего процесса
        if (WIFEXITED(status) && WEXITSTATUS(status) != OK) {
            has_error = WEXITSTATUS(status); // Запоминаем ошибку (если была)
        }
    }

    //while (wait(NULL) > 0);
    return has_error;
}

error find(char const *FindString, char **FindFiles, const char *Files[], const int *CountFiles, int *size, int *capacity) {
    if (!FindString || !FindFiles || !Files || !CountFiles || !size || !capacity) {
        return MEMORY_ERROR;
    }

    int i;
    pid_t pid;
    FILE *file;
    char line[2048];
    char *for_realloc;

    for (i = 0; i < *CountFiles; ++i) {
        pid = fork();

        if (pid < 0) {
            return PROBLEMS_WITH_PID;
        }

        if (pid == 0) {
            file = fopen(Files[i], "r");
            if (!file) {
                exit(PROBLEMS_WITH_FILE);
            }

            while (fgets(line, sizeof(line), file)) {
                if (strstr(line, FindString) != NULL) {
                    if (*size == *capacity) {
                        *capacity *= 2;
                        for_realloc = (char *) realloc (*FindFiles, *capacity * sizeof(char));
                        if (!for_realloc) {
                            fclose(file);
                            return MEMORY_ERROR;
                        }
                        *FindFiles = for_realloc;
                    }

                    (*FindFiles)[*size] = strdup(Files[i]);
                    (*size)++;

                    break;
                }
            }

            fclose(file);
            exit(0);
        }

    }

    while (wait(NULL) > 0);

    return OK;
}


error checkN(const char *N) {
    if (!N) {
        return MEMORY_ERROR;
    }

    int i, len = strlen(N);

    for (i = 4; i < len; ++i) {
        if (!isdigit(N[i])) {
            return NOT_A_NUMBER;
        }
    }

    return OK;
}
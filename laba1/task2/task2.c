#include <ctype.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

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

error FindString(const char **Files, int FileCount, const char *pattern, char *FoundIn, char *FlagFound);

error String_To_uint32_t (const char *str, u_int32_t *result);

error Xor(const char *FileName, int N, u_int64_t *result);

error Mask(const char *FileName, const char *mask, int *count);

int main(int argc, char const *argv[]) {
    int i, N, count;
    char *AnswerString, FlagFound;
    uint64_t XorResult;

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

    if (strncmp(argv[argc - 1], "xor", 3) == 0) {
        N = strtol(argv[argc - 1] + 3, NULL, 10);
        if (N < 2 || N > 6) {
            printf("Invalid input N. N must be >= 2 and <= 6\n");
            return INVALID_INPUT;
        }

        for (i = 1; i < argc - 1; ++i) {
            switch(Xor(argv[i], N, &XorResult)) {
                case MEMORY_ERROR:
                    printf("Memory error\n");
                return MEMORY_ERROR;
                case PROBLEMS_WITH_FILE:
                    printf("Problem with file (%s)\n", argv[i]);
                break;
                default:
                    printf("Xor [%s] result: %lu\n", argv[i], XorResult);
                break;
            }
        }

    } else if (strcmp(argv[argc - 2], "mask") == 0) {
        for (i = 1; i < argc - 2; ++i) {
            switch (Mask(argv[i], argv[argc - 1], &count)) {
                case INVALID_INPUT:
                    printf("Invalid input. Error with %s: \n", argv[argc - 1]);
                    return INVALID_INPUT;
                case MEMORY_ERROR:
                    printf("Memory error\n");
                    return MEMORY_ERROR;
                case PROBLEMS_WITH_FILE:
                    printf("Problem with file (%s)\n", argv[i]);
                    break;
                default:
                    printf("Mask [%s] result: %d\n", argv[i], count);
                    break;
            }
        }
    } else if (strncmp(argv[argc - 1], "copy", 4) == 0) {
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
                        return MEMORY_ERROR;
                    case PROBLEMS_WITH_FILE:
                        printf("Problem with file. %s not processed\n", argv[i]);
                        break;
                    case PROBLEMS_WITH_PID:
                        printf("Problem with pid. %s not processed\n", argv[i]);
                        return PROBLEMS_WITH_PID;
                    default:
                        printf("File %s successful processed\n", argv[i]);
                        break;
                }
            }
        }
    } else if (strncmp(argv[argc - 2], "find", 4) == 0) {
        AnswerString = (char *)malloc(sizeof(char) * argc - 3);
        if (!AnswerString) {
            printf("Memory error\n");
            return MEMORY_ERROR;
        }

        N = argc - 3;

        switch (FindString((const char **)argv + 1, N, argv[argc - 1], AnswerString, &FlagFound)) {
            case MEMORY_ERROR:
                printf("Memory error\n");
                free(AnswerString);
                AnswerString = NULL;
                break;
            case PROBLEMS_WITH_FILE:
                printf("Problem with file\n");
                free(AnswerString);
                AnswerString = NULL;
                break;
            case PROBLEMS_WITH_PID:
                printf("Problem with pid\n");
                free(AnswerString);
                AnswerString = NULL;
                break;
            default:
                printf("Files successful processed\n");

                for (i = 0; i < argc - 3; ++i) {
                    if (FlagFound && AnswerString[i]) {
                        printf("In file: %s string was found\n", argv[i + 1]);
                    } else {
                        printf("In file: %s string was NOT found\n", argv[i + 1]);
                    }
                }

                break;
        }

        free(AnswerString);
        AnswerString = NULL;
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

error FindString(const char **Files, int FileCount, const char *pattern, char *FoundIn, char *FlagFound) {
    if (!Files || !pattern || !FoundIn || !FlagFound) {
        return MEMORY_ERROR;
    }
    
    char *shared;
    char symbol;
    int i, idx = 0, pattern_size = strlen(pattern);
    pid_t pid;
    int shm_id;

    shm_id = shmget(IPC_PRIVATE, (FileCount + 1) * sizeof(char), IPC_CREAT | 0666);

    if (shm_id == -1) {
        return MEMORY_ERROR;
    }
    
    shared = (char *) shmat(shm_id, NULL, 0);
    if (shared == (void *)-1) {
        shmctl(shm_id, IPC_RMID, NULL);
        return MEMORY_ERROR;
    }
    
    memset(shared, 0, FileCount + 1);
    
    for (i = 0; i < FileCount; ++i) {
        pid = fork();

        if (pid < 0) {
            shmdt(shared);
            shmctl(shm_id, IPC_RMID, NULL);
            return PROBLEMS_WITH_PID;
        }
        
        if (pid == 0) {
            FILE *file = fopen(Files[i], "r");
            if (!file) {
                exit(PROBLEMS_WITH_FILE);
            }
            
            while ((symbol = fgetc(file)) != EOF) {
                if (pattern[idx] == symbol) {
                    if (idx == pattern_size - 1) {
                        shared[FileCount] = 1;
                        shared[i] = 1;
                        break;
                    }
                    idx++;
                } else {
                    fseek(file, -idx, SEEK_CUR);
                    idx = 0;
                }
            }

            fclose(file);

            exit(OK);
        }
        
    }

    for (i = 0; i < FileCount; i++) {
        wait(NULL);
    }

    *FlagFound = shared[FileCount];

    memcpy(FoundIn, shared, FileCount * sizeof(char));
    shmdt(shared);
    shmctl(shm_id, IPC_RMID, NULL);

    return OK;
}

error Xor(const char *FileName, int N, u_int64_t *result) {
    if (!FileName || !result) {
        return MEMORY_ERROR;
    }

    uint64_t BlockSIzeBits = 1ULL << N, BitCount;
    uint64_t XorSum = 0, CurrentBlockValue = 0;
    int i, CurrentBit;
    uint8_t byte;
    FILE *file;

    file = fopen(FileName, "rb");
    if (!file) {
        return PROBLEMS_WITH_FILE;
    }

    while (fread(&byte, 1, 1, file) == 1) {
        for (i = 0; i < 8; ++i) {
            CurrentBit = (byte >> i) & 1; // Получаем текущий бит

            CurrentBlockValue |= (CurrentBit << BitCount); // Добавляем бит в блок

            BitCount++;

            if (BitCount == BlockSIzeBits) {
                XorSum ^= CurrentBlockValue;
                CurrentBlockValue = 0;
                BitCount = 0;
            }
        }
    }

    // Обрабатываем последний неполный блок
    if (BitCount > 0) {
        XorSum ^= CurrentBlockValue;
    }

    fclose(file);
    *result = XorSum;
    return OK;
}

error Mask(const char *FileName, const char *mask, int *count) {
    if (!FileName || !mask || !count) {
        return MEMORY_ERROR;
    }

    u_int32_t WriteMask, num32;
    int NewCount = 0;
    size_t tst = 0;
    FILE *file;

    file = fopen(FileName, "rb");
    if (!file) {
        return PROBLEMS_WITH_FILE;
    }

    if (strlen(mask) > 4 || String_To_uint32_t(mask, &WriteMask)) {
        fclose(file);
        return INVALID_INPUT;
    }

    while ((tst == fread(&num32, sizeof(uint32_t), 1, file)) == 1) {
        if (WriteMask & num32) {
            ++NewCount;
        }
    }

    fclose(file);
    *count = NewCount;
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

error String_To_uint32_t (const char *str, u_int32_t *result) {
    if (!str || !result) {
        return MEMORY_ERROR;
    }

    char *endinp;
    unsigned long res;
    res = strtoul(str, &endinp, 16);

    if (res > UINT32_MAX) {
        return INVALID_INPUT;
    }

    if (*endinp != '\0') {
        return INVALID_INPUT;
    }

    *result = (u_int32_t)res;

    return OK;
}
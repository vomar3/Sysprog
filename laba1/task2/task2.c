#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUFFER_SIZE 4096

typedef enum errors {
    OK,
    INVALID_INPUT,
    MEMORY_ERROR,
    FILE_ERROR,
    PID_ERROR,
} error;

error XorN(const char *filename, const int *n, int *result);

error mask(const char *filename, int *result, const int *mask);

error copyN(const char *filename, const int *n);

error find(const char *SearchString, const int *FileCounts, char **FileNames, int *size, int *capacity, char *fileNames[]);

int main(int argc, char const *argv[]) {
    int i, result, n, mask_value, capacity = 2, size = 0;
    char *operation, *value, *FileNames;
    int HowManyFiles;

    if (argc < 3) {
        printf("Enter in the format: ./a.out <paths> <flag>\n");
        return INVALID_INPUT;
    }

    for (i = 0; i < argc; i++) {
        if (argv[i] == NULL) {
            printf("Error with arguments\n");
            return MEMORY_ERROR;
        }
    }

    value = argv[argc - 1];
    operation = argv[argc - 2];

    if (strncmp(value, "xor", 3) == 0) {
        n = strtol(value + 3, NULL, 10);
        for (i = 1; i < argc - 1; i++) {
            switch (XorN(argv[i], &n, &result)) {
                case FILE_ERROR:
                    printf("File error\n");
                    return FILE_ERROR;
                case INVALID_INPUT:
                    printf("Invalid input: N must be >= 2 and <= 6\n");
                    return INVALID_INPUT;
                case MEMORY_ERROR:
                    printf("Memory error\n");
                    return MEMORY_ERROR;
                default:
                    printf("%d", result);
                    break;
            }
        }
    } else if (strcmp(operation, "mask") == 0) {
        mask_value = strtoul(value, NULL, 16);
        for (i = 1; i < argc - 2; i++) {
            switch (mask(argv[i], &result, &mask_value)) {
                case FILE_ERROR:
                    printf("File error\n");
                    return FILE_ERROR;
                case MEMORY_ERROR:
                    printf("Memory error\n");
                    return MEMORY_ERROR;
                default:
                    printf("The number of numbers that fit the mask = %d", result);
                    break;
            }
        }
    } else if (strncmp(value, "copy", 4) == 0) {
        n = strtol(value + 4, NULL, 10);
        for (i = 1; i < argc - 1; i++) {
            switch (copyN(argv[i], &n)) {
                case FILE_ERROR:
                    printf("File error\n");
                    return FILE_ERROR;
                case MEMORY_ERROR:
                    printf("Memory error\n");
                    return MEMORY_ERROR;
                case PID_ERROR:
                    printf("PID error\n");
                    return PID_ERROR;
                default:
                    printf("Copying completed\n");
                    break;
            }
        }
    } else if (strcmp(operation, "find") == 0) {
        FileNames = (char *) malloc (sizeof(char) * capacity);
        if (!FileNames) {
            printf("Memory error\n");
            return MEMORY_ERROR;
        }

        HowManyFiles = argc - 2;

        switch (find(value, &HowManyFiles, &FileNames, &size, &capacity, &argv[1])) {
            case MEMORY_ERROR:
                printf("Memory error\n");
                free(FileNames);
                return MEMORY_ERROR;
            case FILE_ERROR:
                printf("File error\n");
                free(FileNames);
                return FILE_ERROR;
            case PID_ERROR:
                printf("PID error\n");
                free(FileNames);
                return PID_ERROR;
            default:
                if (size == 0) {
                    printf("The specified substring was not found in the files\n");
                } else {
                    printf("Files with the specified substring\n");
                    for (i = 0; i < size; ++i) {
                        printf("%s\n", &FileNames[i]);
                    }
                }
                break;
        }

        free(FileNames);

    } else {
        printf("Undefined command\n");
        return INVALID_INPUT;
    }

    return 0;
}

error XorN(const char *filename, const int *n, int *result) {
    if (!filename || !n || !result) {
        return MEMORY_ERROR;
    }

    short buffer[1 << *n];
    int ReadBytes, i;
    FILE *file;

    if (!(*n >= 2 && *n <= 6)) {
        return INVALID_INPUT;
    }

    file = fopen(filename, "rb");
    if (!file) {
        return FILE_ERROR;
    }

    *result = 0;
    while ((ReadBytes = fread(buffer, 1, (1 << *n), file)) > 0) {
        for (i = 0; i < ReadBytes; i++) {
            *result ^= buffer[i];
        }
    }

    fclose(file);
    return OK;
}

error mask(const char *filename, int *result, const int *mask) {
    if (!filename || !result || !mask) {
        return MEMORY_ERROR;
    }

    int value, count = 0;
    FILE *file = fopen(filename, "rb");
    if (!file) {
        return FILE_ERROR;
    }

    while (fread(&value, sizeof(int), 1, file) == 1) {
        if ((value & *mask) == *mask) {
            count++;
        }
    }

    *result = count;

    fclose(file);
    return OK;
}

error copyN(const char *filename, const int *n) {
    if (!filename || !n) {
        return MEMORY_ERROR;
    }

    int i, bytesRead;
    pid_t pid;
    char newFilename[256];
    FILE *src, *dst;
    char buffer[BUFFER_SIZE];

    for (i = 0; i <= *n; ++i) {
        pid = fork();
        if (pid < 0) {
            return PID_ERROR;
        }

        if (pid == 0) {
            snprintf(newFilename, sizeof(newFilename), "%s_copy%d", filename, i);
            src = fopen(filename, "rb");
            if (!src) {
                return FILE_ERROR;
            }

            dst = fopen(newFilename, "wb");
            if (!dst) {
                fclose(src);
                return FILE_ERROR;
            }
            while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, src)) > 0) {
                fwrite(buffer, 1, bytesRead, dst);
            }

            fclose(src);
            fclose(dst);
        }
    }

    while (wait(NULL) > 0);

    return OK;
}

error find(const char *SearchString, const int *FileCounts, char **FileNames, int *size, int *capacity, char *fileNames[]) {
    if (!SearchString || !FileCounts || !FileNames || !size || !capacity || !fileNames) {
        return MEMORY_ERROR;
    }

    int i, status;
    pid_t pid;
    FILE *file;
    char line[BUFFER_SIZE];
    char *for_realloc;
    for (i = 0; i < *FileCounts; ++i) {
        pid = fork();
        if (pid < 0) {
            return PID_ERROR;
        }

        if (pid == 0) {
            file = fopen(fileNames[i], "rb");
            if (!file) {
                return FILE_ERROR;
            }

            while (fgets(line, sizeof(line), file)) {
                if (strstr(line, SearchString)) {
                    if (*size == *capacity) {
                        for_realloc = (char *) realloc(*fileNames, *capacity *= 2);
                        if (!for_realloc) {
                            return MEMORY_ERROR;
                        }
                        *fileNames = for_realloc;
                    }

                    (*fileNames)[*size] = *fileNames[i];
                    (*size)++;
                    fclose(file);
                }
            }
        }
    }

    for (i = 0; i < *FileCounts; ++i) {
        wait(&status);
    }

    return OK;
}
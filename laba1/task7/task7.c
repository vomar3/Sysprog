#include <complex.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef enum error {
    OK,
    INVALID_INPUT,
    MEMORY_ERROR,
    FILE_ERROR,
    STAT_ERROR,
} error;

typedef struct Files {
    char name[1024];
    unsigned long size;
    unsigned long inode;
    unsigned long blocks;
} files;

error list_files(const char *dirpath, files **my_files, int *capacity, int *size, int *j);

int main(int argc, char *argv[]) {
    int i, capacity = 2, size = 0, j = 0;
    files *my_files = (files *) malloc (sizeof(files) * capacity);

    if (!my_files) {
        perror("malloc");
        return MEMORY_ERROR;
    }

    if (argc < 2) {
        printf("Invalid input. Please enter your catalogs\n");
        return INVALID_INPUT;
    }

    for (i = 0; i < argc; ++i) {
        if (argv[i] == NULL) {
            printf("Memory error\n");
            return MEMORY_ERROR;
        }
    }

    for (i = 1; i < argc; i++) {
        printf("Directory: %s\n", argv[i]);
        switch (list_files(argv[i], &my_files, &capacity, &size, &j)) {
            case MEMORY_ERROR:
                printf("Memory error\n");
                free(my_files);
                return MEMORY_ERROR;
            case FILE_ERROR:
                perror("opendir");
                free(my_files);
                return FILE_ERROR;
            case STAT_ERROR:
                perror("stat");
                free(my_files);
                return STAT_ERROR;
            default:
                for (; j < size; ++j) {
                    printf("%-20s | Size: %-8lu | Inode: %-8lu | First block: %-8lu\n",
                        my_files[j].name, my_files[j].size, my_files[j].inode, my_files[j].blocks);
                }
        }
    }

    free(my_files);
    return 0;
}

error list_files(const char *dirpath, files **my_files, int *capacity, int *size, int *j) {
    if (!dirpath || !my_files || !capacity || !size || !j) {
        return MEMORY_ERROR;
    }

    *j = *size;
    DIR *dir = opendir(dirpath);
    // Хранит информацию о записях в каталоге
    struct dirent *entry;
    files *for_realloc;
    // Тут будут храниться метаданные о файле
    struct stat st;
    char path[1024];

    if (!dir) {
        return FILE_ERROR;
    }

    // Идем до конца каталога по одной записи
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.' || strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        if (*size == *capacity) {
            *capacity *= 2;
            for_realloc = (files *) realloc(*my_files, *capacity * sizeof (files));
            if (!for_realloc) {
                return MEMORY_ERROR;
            }

            *my_files = for_realloc;
        }

        snprintf(path, sizeof(path), "%s/%s", dirpath, entry->d_name);

        // Если метаданные о файле не собрались, то увы
        if (stat(path, &st) == -1) {
            return STAT_ERROR;
        }

        strcpy((*my_files)[*size].name, entry->d_name);
        (*my_files)[*size].size = st.st_size;
        (*my_files)[*size].inode = st.st_ino;
        (*my_files)[*size].blocks = st.st_blocks;

        (*size)++;
    }

    closedir(dir);

    return OK;
}
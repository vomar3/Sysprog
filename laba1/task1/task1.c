#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>

typedef enum errors {
    OK,
    INVALID_INPUT,
    MEMORY_ERROR,
    FILE_ERROR,
    NOT_FOUND,
    FOUND,
} error;

typedef struct user{
    char login[7];
    unsigned int Pin_code;
    int sanctions;
} user;

error check_correct_login(const char *login);

error check_correct_PIN(unsigned int Pin);

void clear_input_buffer();

error registration_login(user **users, const int *count_of_user, int *capacity, char *reg_name);

void Time();

void Date();

error howmuch_command(const char *date_str, const char *flag, double *result);

void free_users(user *users);

error login(user **users, const int *count_of_users, int *id, const unsigned int *pass, const char *login_name);

void main_menu();

void lower_menu();

error Sanctions(user **users, const char *name, int count_user, int current_id, int *j);

error save_users_to_file(user *users, int count_of_users);

error load_users_from_file(user **users, int *count_of_users, int *capacity);

error update_sanctions_in_file(const char *filename, const char *login, int new_sanctions);

int main() {
    char flag[2], user_name[7], login_name[7], reg_name[7] = {};
    char check_input[10], time[10];
    int current_id = -1, check = 0, exit = 1, number, j = -1;
    int reg, capacity = 2, user_count = 0;
    int for_check_correct_write = 1;
    unsigned int reg_pass, answer, pass;
    bool check_reg = false;
    double result;
    user *users = (user *) malloc (capacity * sizeof(user));
    if (!users) {
        printf("Memory error\n");
        return MEMORY_ERROR;
    }

    // Загрузка всех юзеров, если они есть
    if (load_users_from_file(&users, &user_count, &capacity) != OK) {
        printf("Error loading users from file\n");
        return FILE_ERROR;
    }

    lower_menu();
    while (scanf("%d", &reg) != 1) {
        printf("Invalid input\n");
        clear_input_buffer();
        lower_menu();
    }

    while (exit) {
        switch (reg) {
            case 1:
                // Регистрация + Перекидывание юзера в файл

                do {
                    printf("Write your login for registration: \n");
                    scanf("%7s", reg_name);
                    clear_input_buffer();

                    if (reg_name[6] != '\0') {
                        printf("Incorrect input. \n");
                        reg_name[6] = '\0';
                        continue;
                    }

                    if (registration_login(&users, &user_count, &capacity, reg_name) == FOUND) {
                        printf("This login already exists\n");
                        continue;
                    }

                    if (check_correct_login(reg_name) != OK) {
                        printf("Incorrect input. \n");
                    } else {
                        check_reg = true;
                    }
                } while (check_reg == false);
                check_reg = false;

                do {
                    printf("Write your password for registration: \n");
                    if (scanf("%u", &reg_pass) != 1) {
                        printf("Invalid input\n");
                        clear_input_buffer();
                        continue;
                    }

                    clear_input_buffer();
                    if (check_correct_PIN(reg_pass) != OK) {
                        printf("Incorrect input. \n");
                    } else {
                        check_reg = true;
                    }
                } while (check_reg == false);

                strcpy((users)[user_count].login, reg_name);
                (users)[user_count].Pin_code = reg_pass;
                (users)[user_count].sanctions = -3;
                ++(user_count);

                printf("Registration successful\n");

                if (save_users_to_file(users, user_count) != OK) {
                    printf("Error opening file for saving users\n");
                }

                lower_menu();
                while (scanf("%d", &reg) != 1) {
                    printf("Invalid input\n");
                    clear_input_buffer();
                    lower_menu();
                }
                break;
            case 2:
                printf("Enter your login for authorization: ");
                scanf("%6s", login_name);
                clear_input_buffer();
                printf("Enter your Pin-code for authorization: ");
                scanf("%u", &pass);
                clear_input_buffer();

                if (login(&users, &user_count, &current_id, &pass, login_name) != OK) {
                    printf("The user with this username or password was not found\n");
                    lower_menu();
                    scanf("%d", &reg);
                    break;
                } else {
                    printf("Login successful\n");
                }

                // До тех пор, пока не выход из аккаунта или количество операций (sanctions) не станет 0
                while (check == 0 && users[current_id].sanctions != 0) {
                    main_menu();
                    scanf("%s", check_input);
                    --users[current_id].sanctions;

                    if (strcmp(check_input, "Time") == 0) {
                        Time();
                    } else if (strcmp(check_input, "Date") == 0) {
                        Date();
                    } else if (strcmp(check_input, "Howmuch") == 0) {
                        if (scanf("%10s %2s", time, flag) != 2) {
                            printf("Invalid input\n");
                        }
                        clear_input_buffer();
                        if (howmuch_command(time, flag, &result) != OK) {
                            printf("Error input\n");
                        } else {
                            printf("Past tense is %lf\n", result);
                        }
                    } else if (strcmp(check_input, "Logout") == 0) {
                        check = 1;
                    } else if (strcmp(check_input, "Sanctions") == 0) {
                        //printf("Enter a username and a limit on the number of commands\n");
                        if (scanf("%7s %d", user_name, &number) != 2) {
                            printf("Invalid input\n");
                        }

                        clear_input_buffer();

                        if (Sanctions(&users, user_name, user_count, current_id, &j) == FOUND) {

                            printf("Write 12345 to confirm the action\n");
                            scanf("%u", &answer);

                            if (answer != 12345) {
                                printf("Error input\n");
                            } else {
                                users[j].sanctions = number;
                                printf("Successful\n");
                                if (update_sanctions_in_file("logins.txt", user_name, number) != OK) {
                                    printf("Error with file\n");
                                }
                            }

                        } else {
                            printf("Error\n");
                        }
                    } else {
                        printf("Try again\n");
                    }
                }

                check = 0;
                lower_menu();
                while (scanf("%d", &reg) != 1) {
                    printf("Invalid input\n");
                    clear_input_buffer();
                    lower_menu();
                }
                break;
            case 3:
                exit = 0;
                break;
            default:
                printf("Try again\n");

                lower_menu();
                while (scanf("%d", &reg) != 1) {
                    printf("Invalid input\n");
                    clear_input_buffer();
                    lower_menu();
                }
                break;
        }
    }

    free_users(users);
    return 0;
}

error check_correct_login(const char *login) {
    if (!login) {
        return MEMORY_ERROR;
    }

    size_t size = strlen(login), i;

    for (i = 0; i < size; ++i) {
        if (!isalnum(login[i])) {
            return INVALID_INPUT;
        }
    }

    return OK;
}

error check_correct_PIN(unsigned int Pin) {
    if (Pin > 100000) {
        return INVALID_INPUT;
    }

    return OK;
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

error registration_login(user **users, const int *count_of_user, int *capacity, char *reg_name) {
    if (!users || !count_of_user || !capacity || !reg_name) {
        return MEMORY_ERROR;
    }

    int i;
    user *realloc_user;

    if (*count_of_user == *capacity) {
        *capacity *= 2;
        realloc_user = (user *) realloc(*users, *capacity * sizeof (user));
        if (!realloc_user) {
            return MEMORY_ERROR;
        }
        *users = realloc_user;
    }

    for (i = 0; i < *count_of_user; ++i) {
        if (strcmp((*users)[i].login, reg_name) == 0) {
            return FOUND;
        }
    }

    return NOT_FOUND;
}

void Time() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("Current time: %02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void Date() {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    printf("Current date: %04d-%02d-%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
}

void free_users(user *users) {
    free(users);
}

error login(user **users, const int *count_of_users, int *id, const unsigned int *pass, const char *login_name) {
    if (!users || !count_of_users || !pass || !login_name) {
        return MEMORY_ERROR;
    }

    int i;

    for (i = 0; i < *count_of_users; ++i) {
        if ((*users)[i].Pin_code == *pass && strcmp((*users)[i].login, login_name) == 0) {
            *id = i;
            return OK;
        }
    }

    return NOT_FOUND;
}

error Sanctions(user **users, const char *name, int count_user, int current_id, int *j) {
    if (!users || !name || !j) {
        return MEMORY_ERROR;
    }

    int i;

    for (i = 0; i < count_user; ++i) {
        // Проверка current_id != i для того, чтобы исключить возможность навесить санкции на себя
        if (strcmp((*users)[i].login, name) == 0 && current_id != i) {
            *j = i;
            return FOUND;
        }
    }

    return NOT_FOUND;
}

void main_menu() {
    printf("---------------------------------------------------------------------------------------------\n");
    printf("Write the following action in text.\n"
           "Time - Output the current time\n"
           "Date - Output the current date\n"
           "Howmuch time flag - View the elapsed time in format YYYY-MM-DD and the flag -s, -m, -h, -y\n"
           "Logout - Logout\n"
           "Sanctions username number - Set a limit on the number of operations\n");
    printf("---------------------------------------------------------------------------------------------\n");
}

void lower_menu() {
    printf("------------------------------------------------\n");
    printf("1 - sign up, 2 - log in, 3 - exit\n");
    printf("------------------------------------------------\n");
}

error save_users_to_file(user *users, int count_of_users) {
    if (!users) {
        return MEMORY_ERROR;
    }

    int i;
    FILE *file = fopen("logins.txt", "w");

    if (!file) {
        return FILE_ERROR;
    }

    for (i = 0; i < count_of_users; i++) {
        fprintf(file, "%s %u %d\n", users[i].login, users[i].Pin_code, users[i].sanctions);
    }

    fclose(file);

    return OK;
}

error load_users_from_file(user **users, int *count_of_users, int *capacity) {
    if (!users || !count_of_users || !capacity) {
        return MEMORY_ERROR;
    }

    FILE *file = fopen("logins.txt", "r");
    if (!file) {
        return FILE_ERROR;
    }

    user temp;
    while (fscanf(file, "%s %u %d", temp.login, &temp.Pin_code, &temp.sanctions) == 3) {
        if (*count_of_users == *capacity) {
            *capacity *= 2;
            user *new_users = (user *) realloc(*users, (*capacity) * sizeof(user));
            if (!new_users) {
                fclose(file);
                return MEMORY_ERROR;
            }
            *users = new_users;
        }
        (*users)[(*count_of_users)++] = temp;
    }

    fclose(file);
    return OK;
}

error update_sanctions_in_file(const char *filename, const char *login, int new_sanctions) {
    if (!filename || !login) {
        return MEMORY_ERROR;
    }

    FILE *file = fopen(filename, "r");
    if (!file) {
        return FILE_ERROR;
    }

    // Временный файл, чтобы перезаписать данные с помощью него
    FILE *temp = fopen("temp.txt", "w");
    if (!temp) {
        fclose(file);
        return FILE_ERROR;
    }

    user temp_user;
    while (fscanf(file, "%s %u %d", temp_user.login, &temp_user.Pin_code, &temp_user.sanctions) == 3) {
        if (strcmp(temp_user.login, login) == 0) {
            temp_user.sanctions = new_sanctions;
        }
        fprintf(temp, "%s %u %d\n", temp_user.login, temp_user.Pin_code, temp_user.sanctions);
    }

    fclose(file);
    fclose(temp);
    remove(filename);
    rename("temp.txt", filename);

    return OK;
}

error howmuch_command(const char *date_str, const char *flag, double *result) {
    if (!flag || !result || !date_str) {
        return MEMORY_ERROR;
    }

    struct tm input_time = {0};
    time_t current_time, input_time_t;
    double seconds_diff;

    if (sscanf(date_str, "%d-%d-%d", &input_time.tm_year, &input_time.tm_mon, &input_time.tm_mday) != 3) {
        return INVALID_INPUT;
    }

    if (input_time.tm_mon > 12 || input_time.tm_mon < 1 || input_time.tm_mday < 1
        || input_time.tm_mday > 31 || input_time.tm_year < 1900) {
        return INVALID_INPUT;
    }

    if (input_time.tm_mon == 2) {
        if (input_time.tm_mday > 29) {
            return INVALID_INPUT;
        }

        if (input_time.tm_mday == 29) {
            if (!(input_time.tm_year % 4 == 0 && (input_time.tm_year % 100 != 0 || input_time.tm_year % 400 == 0))) {
                return INVALID_INPUT;
            }
        }
    }

    if (input_time.tm_mday == 31) {
        if (input_time.tm_mon == 4 || input_time.tm_mon == 6 || input_time.tm_mon == 9 || input_time.tm_mon == 11) {
            return INVALID_INPUT;
        }
    }

    input_time.tm_year -= 1900;
    input_time.tm_mon -= 1;

    input_time_t = mktime(&input_time);
    if (input_time_t == -1) {
        return INVALID_INPUT;
    }

    current_time = time(NULL);
    if (current_time == -1) {
        return INVALID_INPUT;
    }

    seconds_diff = difftime(current_time, input_time_t);
    if (seconds_diff < 0) {
        return INVALID_INPUT;
    }

    if (strcmp(flag, "-s") == 0) {
        *result = seconds_diff;
    } else if (strcmp(flag, "-m") == 0) {
        *result = seconds_diff / 60;
    } else if (strcmp(flag, "-h") == 0) {
        *result = seconds_diff / 3600;
    } else if (strcmp(flag, "-y") == 0) {
        *result = seconds_diff / (3600 * 24 * 365.25);
    } else {
        return INVALID_INPUT;
    }

    return OK;
}
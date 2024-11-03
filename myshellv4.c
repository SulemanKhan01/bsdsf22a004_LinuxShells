#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

#define HISTORY_SIZE 10
#define MAX_COMMAND_LENGTH 1024

char* history[HISTORY_SIZE];
int history_count = 0;

void add_to_history(const char* command) {
    if (history_count == HISTORY_SIZE) {
        free(history[0]);
        for (int i = 1; i < HISTORY_SIZE; i++) {
            history[i - 1] = history[i];
        }
        history_count--;
    }
    history[history_count++] = strdup(command);
}

void display_history() {
    for (int i = 0; i < history_count; i++) {
        printf("%d: %s\n", i + 1, history[i]);
    }
}

char* get_command_from_history(int index) {
    if (index < 1 || index > history_count) {
        fprintf(stderr, "Error: No command found for !%d\n", index);
        return NULL;
    }
    return history[index - 1];
}

void execute_command(char* command) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork failed");
    } else if (pid == 0) {
        char* args[MAX_COMMAND_LENGTH / 2 + 1];
        int arg_count = 0;

        char* token = strtok(command, " \t\n");
        while (token != NULL) {
            args[arg_count++] = token;
            token = strtok(NULL, " \t\n");
        }
        args[arg_count] = NULL;

        if (execvp(args[0], args) == -1) {
            perror("execvp failed");
            exit(EXIT_FAILURE);
        }
    } else {
        wait(NULL);
    }
}

int main() {
    char command[MAX_COMMAND_LENGTH];

    while (1) {
        printf("SULEMANshell: ");
        fflush(stdout);

        if (!fgets(command, sizeof(command), stdin)) {
            printf("\nExiting shell...\n");
            break;
        }

        command[strcspn(command, "\n")] = 0;

        if (strlen(command) == 0) continue;

        if (command[0] == '!') {
            char* recalled_command = NULL;

            // Check for !!
            if (strcmp(command, "!!") == 0) {
                if (history_count == 0) {
                    fprintf(stderr, "Error: No commands in history\n");
                    continue;
                }
                recalled_command = history[history_count - 1];
            } else {
                int cmd_num = atoi(&command[1]);
                recalled_command = get_command_from_history(cmd_num);
            }

            if (recalled_command) {
                printf("Repeating command: %s\n", recalled_command);
                add_to_history(recalled_command);
                execute_command(recalled_command);
            }
        } else {
            add_to_history(command);
            execute_command(command);
        }
    }

    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }

    return 0;
}
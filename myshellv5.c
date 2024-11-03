#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <limits.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 100
#define MAX_JOBS 100

typedef struct {
    pid_t pid;
    char command[MAX_INPUT_SIZE];
} Job;

Job jobs[MAX_JOBS];
int jobCount = 0;

// Function to display the prompt
void displayPrompt() {
    printf("SULEMANshell: ");
}

// Function to execute built-in commands
int executeBuiltInCommand(char **args) {
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "SULEMANshell: expected argument to \"cd\"\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("SULEMANshell");
            }
        }
        return 1;
    } else if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "jobs") == 0) {
        for (int i = 0; i < jobCount; i++) {
            printf("[%d] %d %s\n", i + 1, jobs[i].pid, jobs[i].command);
        }
        return 1;
    } else if (strcmp(args[0], "kill") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "SULEMANshell: expected argument to \"kill\"\n");
        } else {
            int jobNum = atoi(args[1]);
            if (jobNum > 0 && jobNum <= jobCount) {
                if (kill(jobs[jobNum - 1].pid, SIGKILL) == 0) {
                    printf("Job [%d] with PID %d terminated\n", jobNum, jobs[jobNum - 1].pid);
                } else {
                    perror("SULEMANshell");
                }
            } else {
                fprintf(stderr, "SULEMANshell: invalid job number\n");
            }
        }
        return 1;
    } else if (strcmp(args[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("cd <directory> - change the current directory\n");
        printf("exit - exit the shell\n");
        printf("jobs - list background jobs\n");
        printf("kill <job_number> - terminate a background job\n");
        printf("help - show this help message\n");
        return 1;
    }
    return 0;
}

// Function to parse the command
void parseCommand(char *input, char **args) {
    char *token = strtok(input, " \t\r\n\a");
    int position = 0;
    while (token != NULL && position < MAX_ARGS - 1) {
        args[position++] = token;
        token = strtok(NULL, " \t\r\n\a");
    }
    args[position] = NULL;
}

// Function to execute external commands
void executeExternalCommand(char **args, int isBackground) {
    pid_t pid = fork();
    if (pid < 0) {
        perror("SULEMANshell");
    } else if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("SULEMANshell");
        }
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        if (!isBackground) {
            waitpid(pid, NULL, 0);  // Wait for the foreground process
        } else {
            // Save job details for background process
            jobs[jobCount].pid = pid;
            strcpy(jobs[jobCount].command, args[0]);
            jobCount++;
            printf("Job [%d] started with PID %d\n", jobCount, pid);
        }
    }
}

// Main loop of the shell
void shellLoop() {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARGS];

    while (1) {
        displayPrompt();
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            break;
        }

        // Remove trailing newline character
        input[strcspn(input, "\n")] = 0;

        parseCommand(input, args);

        // Check if the last argument is "&" for background execution
        int isBackground = 0;
        if (args[0] != NULL) {
            int lastArgIndex = 0;
            while (args[lastArgIndex] != NULL) {
                lastArgIndex++;
            }
            if (lastArgIndex > 0 && strcmp(args[lastArgIndex - 1], "&") == 0) {
                args[lastArgIndex - 1] = NULL; // Remove "&"
                isBackground = 1;
            }
        }

        if (args[0] == NULL) {
            continue;
        }

        if (!executeBuiltInCommand(args)) {
            executeExternalCommand(args, isBackground);
        }
    }
}

int main() {
    shellLoop();
    return 0;
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "SULEMANshell:- "

int execute(char* arglist[]);
char** tokenize(char* cmdline);
char* read_cmd(char*, FILE*);
int handle_redirection_and_pipes(char* arglist[]);

int main() {
    char *cmdline;
    char** arglist;
    char* prompt = PROMPT;

    while ((cmdline = read_cmd(prompt, stdin)) != NULL) {
        if ((arglist = tokenize(cmdline)) != NULL) {
            if (handle_redirection_and_pipes(arglist) == 0) {
                execute(arglist);
            }

            // Safely free arglist
            for (int j = 0; j < MAXARGS + 1; j++) {
                if (arglist[j] != NULL) {
                    free(arglist[j]);
                }
            }
            free(arglist);
            free(cmdline);
        }
    }
    printf("\n");
    return 0;
}

int handle_redirection_and_pipes(char* arglist[]) {
    int in_fd = -1, out_fd = -1;
    int pipe_fd[2];
    int orig_stdin = dup(STDIN_FILENO); // Save original stdin
    int orig_stdout = dup(STDOUT_FILENO); // Save original stdout
    int i;

    for (i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "<") == 0 && arglist[i + 1] != NULL) {
            in_fd = open(arglist[i + 1], O_RDONLY);
            if (in_fd < 0) {
                perror("Failed to open input file");
                return -1;
            }
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
            arglist[i] = NULL;
            break;
        } else if (strcmp(arglist[i], ">") == 0 && arglist[i + 1] != NULL) {
            out_fd = open(arglist[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (out_fd < 0) {
                perror("Failed to open output file");
                return -1;
            }
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
            arglist[i] = NULL;
            break;
        } else if (strcmp(arglist[i], "|") == 0) {
            arglist[i] = NULL;
            if (pipe(pipe_fd) == -1) {
                perror("Pipe failed");
                return -1;
            }
            int pid1 = fork();
            if (pid1 == 0) {
                close(pipe_fd[0]);
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[1]);
                execute(arglist); // Execute left side of pipe
                exit(0);
            }
            int pid2 = fork();
            if (pid2 == 0) {
                close(pipe_fd[1]);
                dup2(pipe_fd[0], STDIN_FILENO);
                close(pipe_fd[0]);
                execute(&arglist[i + 1]); // Execute right side of pipe
                exit(0);
            }
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            dup2(orig_stdin, STDIN_FILENO);  // Restore original stdin
            dup2(orig_stdout, STDOUT_FILENO); // Restore original stdout
            close(orig_stdin);
            close(orig_stdout);
            return 1;
        }
    }

    // Restore original stdin and stdout if redirection was used
    if (in_fd != -1 || out_fd != -1) {
        dup2(orig_stdin, STDIN_FILENO);
        dup2(orig_stdout, STDOUT_FILENO);
        close(orig_stdin);
        close(orig_stdout);
    }
    return 0;
}

int execute(char* arglist[]) {
    int status;
    int cpid = fork();

    switch (cpid) {
        case -1:
            perror("fork failed");
            exit(1);
        case 0:
            execvp(arglist[0], arglist);
            perror("Command not found...");
            exit(1);
        default:
            waitpid(cpid, &status, 0);
            printf("child exited with status %d\n", status >> 8);
            return 0;
    }
}

char** tokenize(char* cmdline) {
    char** arglist = (char**)malloc(sizeof(char*) * (MAXARGS + 1));
    for (int j = 0; j < MAXARGS + 1; j++) {
        arglist[j] = (char*)malloc(sizeof(char) * ARGLEN);
        bzero(arglist[j], ARGLEN);
    }

    if (cmdline[0] == '\0') 
        return NULL;

    int argnum = 0;
    char* cp = cmdline;
    char* start;
    int len;

    while (*cp != '\0' && argnum < MAXARGS) {
        while (*cp == ' ' || *cp == '\t') cp++;
        start = cp;
        len = 1;
        while (*++cp != '\0' && !(*cp == ' ' || *cp == '\t'))
            len++;
        strncpy(arglist[argnum], start, len);
        arglist[argnum][len] = '\0';
        argnum++;
    }
    arglist[argnum] = NULL; // NULL-terminate for execvp compatibility
    return arglist;
}

char* read_cmd(char* prompt, FILE* fp) {
    printf("%s", prompt);
    int c, pos = 0;
    char* cmdline = (char*) malloc(sizeof(char) * MAX_LEN);
    while ((c = getc(fp)) != EOF) {
        if (c == '\n') break;
        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0)
        return NULL;
    cmdline[pos] = '\0';
    return cmdline;
}

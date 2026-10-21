#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_ARGS 64

int main() {
    char *line = NULL;
    size_t bufsize = 0;
    
    while(1) {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("noor-shell:%s> ", cwd);
        } else {
            printf("noor-shell> ");
        }
        fflush(stdout);
        
        if (getline(&line, &bufsize, stdin) == -1) {
            break;
        }
        
        line[strcspn(line, "\n")] = 0;
        
        if (strlen(line) == 0) {
            continue;
        }
        
        if (strcmp(line, "exit") == 0) {
            break;
        }

        if (strcmp(line, "cd") == 0 || strncmp(line, "cd ", 3) == 0) {
            char *path = NULL;
            if (strcmp(line, "cd") == 0) {
                path = getenv("HOME");
                if (path == NULL) {
                    fprintf(stderr, "noor-shell: HOME not set\n");
                    continue;
                }
            } else {
                path = line + 3;
            }
            
            if (chdir(path) != 0) {
                perror("noor-shell (cd failed)");
            }
            continue;
        }

        char *args[MAX_ARGS];
        int i = 0;
        char *token = strtok(line, " ");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        if (args[0] == NULL) {
            continue;
        }

        int background = 0;
        if (i > 0 && strcmp(args[i - 1], "&") == 0) {
            background = 1;
            args[i - 1] = NULL;
        }

        // Check for a pipe '|'
        int pipe_idx = -1;
        for (int j = 0; args[j] != NULL; j++) {
            if (strcmp(args[j], "|") == 0) {
                pipe_idx = j;
                args[j] = NULL; // Split arguments at the pipe
                break;
            }
        }

        if (pipe_idx != -1) {
            // Handle Piping: cmd1 [args] | cmd2 [args]
            char **left_args = args;
            char **right_args = &args[pipe_idx + 1];

            int pipefds[2];
            if (pipe(pipefds) < 0) {
                perror("noor-shell (pipe failed)");
                continue;
            }

            pid_t pid1 = fork();
            if (pid1 == 0) {
                // Left child writes to pipe
                dup2(pipefds[1], STDOUT_FILENO);
                close(pipefds[0]);
                close(pipefds[1]);
                if (execvp(left_args[0], left_args) == -1) {
                    perror("noor-shell");
                }
                exit(1);
            }

            pid_t pid2 = fork();
            if (pid2 == 0) {
                // Right child reads from pipe
                dup2(pipefds[0], STDIN_FILENO);
                close(pipefds[0]);
                close(pipefds[1]);
                if (execvp(right_args[0], right_args) == -1) {
                    perror("noor-shell");
                }
                exit(1);
            }

            close(pipefds[0]);
            close(pipefds[1]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);

        } else {
            // Standard execution with redirection
            char *infile = NULL;
            char *outfile = NULL;
            for (int j = 0; args[j] != NULL; j++) {
                if (strcmp(args[j], "<") == 0) {
                    args[j] = NULL;
                    if (args[j + 1] != NULL) infile = args[j + 1];
                } else if (strcmp(args[j], ">") == 0) {
                    args[j] = NULL;
                    if (args[j + 1] != NULL) outfile = args[j + 1];
                }
            }

            pid_t pid = fork();
            if (pid < 0) {
                perror("fork failed");
                exit(1);
            } else if (pid == 0) {
                if (infile != NULL) {
                    int fd_in = open(infile, O_RDONLY);
                    if (fd_in < 0) {
                        perror("noor-shell (input open failed)");
                        exit(1);
                    }
                    dup2(fd_in, STDIN_FILENO);
                    close(fd_in);
                }

                if (outfile != NULL) {
                    int fd_out = open(outfile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd_out < 0) {
                        perror("noor-shell (output open failed)");
                        exit(1);
                    }
                    dup2(fd_out, STDOUT_FILENO);
                    close(fd_out);
                }

                if (execvp(args[0], args) == -1) {
                    perror("noor-shell");
                }
                exit(1);
            } else {
                if (!background) {
                    int status;
                    waitpid(pid, &status, 0);
                } else {
                    printf("[Background PID: %d]\n", pid);
                }
            }
        }
    }
    
    free(line);
    return 0;
}

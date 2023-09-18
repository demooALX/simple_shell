#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>

#define MAX_INPUT_LENGTH 1024

/*
*&& and || in your shell to execute commands
*   conditionally based on the success or failure of previous commands. 
*execute_command: function to execute a single command
*parse_command: function that allows for parsing
*custom_getline: function that allows to get line
*command_exists: function that check for if path is present 
*main: where the main function is executed
*printf: display the prompt
*token: Tokenize the input to separate the command and arguments
*Return: string output to the screen
*/

void parse_command(char *input, char **command, char **args){
    int i = 0;
    while (input[i] != '\0' && input[i] != ' ' && input[i] != '\t' && input[i] != '\n'){
        i++;
    }

    *command = strndup(input, i);

    while (input[i] == ' ' || input[i] == '\t'){
        i++;
    }

    int arg_count = 0;
    while (input[i] != '\0' && input[i] != '\n'){
        int arg_start = i;
        while (input[i] != '\0' && input[i] != ' ' && input[i] != '\t' && input[i] != '\n'){
            i++;
        }
        args[arg_count++] = strndup(input + arg_start, i - arg_start);

        while (input[i] == ' ' || input[i] == '\t'){
            i++;
        }
    }

    args[arg_count] = NULL;
}


int execute_command(char *command, char **args) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }
    
    else if (pid == 0) {
        if (execvp(command, args) == -1) {
            perror("execvp");
            exit(EXIT_FAILURE);
        }
    } 
    
    else {
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
    }

    return -1;
}

int main(void) {
    char input[MAX_INPUT_LENGTH];
    char cwd[PATH_MAX]; 

    while (1) {
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
            exit(EXIT_FAILURE);
        }

        printf("simple_shell:%s$ ", cwd); 
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) {
            continue;
        }

        char *commands[MAX_INPUT_LENGTH];
        char *token = strtok(input, ";");

        int num_commands = 0;
        while (token != NULL) {
            commands[num_commands++] = strdup(token);
            token = strtok(NULL, ";");
        }

        int status = 0;

        for (int i = 0; i < num_commands; i++) {
            char *command;
            char *args[MAX_INPUT_LENGTH];

            parse_command(commands[i], &command, args);

            if (strcmp(command, "exit") == 0) {
                int exit_status = 0;
                if (args[1] != NULL) {
                    exit_status = atoi(args[1]);
                }
                printf("Exiting simple_shell with status %d.\n", exit_status);
                exit(exit_status);
            }
            
            else if (strcmp(command, "setenv") == 0) {
                if (args[1] != NULL && args[2] != NULL) {
                    if (setenv(args[1], args[2], 1) != 0) {
                        fprintf(stderr, "Failed to set environment variable %s\n", args[1]);
                    }
                }
                
                else {
                    fprintf(stderr, "Usage: setenv VARIABLE VALUE\n");
                }
            }
            
            else if (strcmp(command, "unsetenv") == 0) {
                if (args[1] != NULL) {
                    if (unsetenv(args[1]) != 0) {
                        fprintf(stderr, "Failed to unset environment variable %s\n", args[1]);
                    }
                }
                
                else {
                    fprintf(stderr, "Usage: unsetenv VARIABLE\n");
                }
            }
            
            else if (strcmp(command, "cd") == 0) {
                if (args[1] == NULL || strcmp(args[1], "~") == 0) {
                    if (chdir(getenv("HOME")) != 0) {
                        perror("chdir");
                    }
                }
                
                else if (strcmp(args[1], "-") == 0) {
                    char *prev_dir = getenv("OLDPWD");
                    if (prev_dir != NULL && chdir(prev_dir) != 0) {
                        perror("chdir");
                    }
                }
                
                else {
                    if (chdir(args[1]) != 0) {
                        perror("chdir");
                    }
                }

                if (setenv("PWD", getcwd(cwd, sizeof(cwd)), 1) != 0) {
                    perror("setenv");
                }
            }
            
            else {
                if (status == 0) {
                    if (strcmp(command, "&&") == 0) {
                        status = execute_command(args[0], args + 1);
                    }
                    
                    else if (strcmp(command, "||") == 0) {
                        status = execute_command(args[0], args + 1);
                    }
                    
                    else {
                        status = execute_command(command, args);
                    }
                }
                
                else {
                    status = 0;
                }
            }

            free(command);
            for (int j = 0; args[j] != NULL; j++) {
                free(args[j]);
            }
        }

        for (int i = 0; i < num_commands; i++) {
            free(commands[i]);
        }
    }

    printf("Exiting simple_shell.\n");
    return 0;
}


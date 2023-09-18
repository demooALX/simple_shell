#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>

#define MAX_INPUT_LENGTH 1024

/*
*PWD: Environment varible
*chdir: functions that is use to change directory
*getcwd: function that is use to change directory
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


int main(void){
    char input[MAX_INPUT_LENGTH];
    char cwd[PATH_MAX];
    while (1){
        if (getcwd(cwd, sizeof(cwd)) == NULL){
            perror("getcwd");
            exit(EXIT_FAILURE);
        }

        printf("simple_shell:%s$ ", cwd);

        if (fgets(input, sizeof(input), stdin) == NULL){
           
            printf("\n");
            break;
        }
        
        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0){
            continue;
        }

        char *command;
        char *args[MAX_INPUT_LENGTH];

        parse_command(input, &command, args);

        if (strcmp(command, "exit") == 0){
            int exit_status = 0;
            if (args[1] != NULL){
                exit_status = atoi(args[1]);
            }
            printf("Exiting simple_shell with status %d.\n", exit_status);
            exit(exit_status);
        }
        
        else if (strcmp(command, "setenv") == 0){
            if (args[1] != NULL && args[2] != NULL){
                if (setenv(args[1], args[2], 1) != 0){
                    fprintf(stderr, "Failed to set environment variable %s\n", args[1]);
                }
            }
            
            else{
                fprintf(stderr, "Usage: setenv VARIABLE VALUE\n");
            }
        }
        
        else if (strcmp(command, "unsetenv") == 0){
            if (args[1] != NULL){
                if (unsetenv(args[1]) != 0){
                    fprintf(stderr, "Failed to unset environment variable %s\n", args[1]);
                }
            } else {
                fprintf(stderr, "Usage: unsetenv VARIABLE\n");
            }
        }
        
        else if (strcmp(command, "cd") == 0){
            if (args[1] == NULL || strcmp(args[1], "~") == 0){
                if (chdir(getenv("HOME")) != 0) {
                    perror("chdir");
                }
            }
            
            else if (strcmp(args[1], "-") == 0){
                char *prev_dir = getenv("OLDPWD");
                if (prev_dir != NULL && chdir(prev_dir) != 0){
                    perror("chdir");
                }
            }
            
            else{
                if (chdir(args[1]) != 0){
                    perror("chdir");
                }
            }

            if (setenv("PWD", getcwd(cwd, sizeof(cwd)), 1) != 0){
                perror("setenv");
            }
        }
        
        else{
            pid_t pid = fork();

            if (pid == -1){
                perror("fork");
                exit(EXIT_FAILURE);
            }
            
            else if (pid == 0){

                if (execvp(command, args) == -1){
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }
            }
            
            else{
                int status;
                if (waitpid(pid, &status, 0) == -1){
                    perror("waitpid");
                    exit(EXIT_FAILURE);
                }

                if (WIFEXITED(status)){
                    int exit_status = WEXITSTATUS(status);
                    if (exit_status != 0){
                        fprintf(stderr, "Command exited with status %d\n", exit_status);
                    }
                }
                
                else if (WIFSIGNALED(status)){
                    fprintf(stderr, "Command terminated by signal %d\n", WTERMSIG(status));
                }
            }
        }

        free(command);
        for (int i = 0; args[i] != NULL; i++){
            free(args[i]);
        }
    }

    printf("Exiting simple_shell.\n");
    return 0;
}


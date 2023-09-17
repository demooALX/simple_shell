i#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_LENGTH 1024


/*
*command_exists: function that check for if path is present 
*main: where the main function is executed
*printf: display the prompt
*token: Tokenize the input to separate the command and arguments
*Return: string output to the screen
*/

int command_exists(const char *command){
    char *path_env = getenv("PATH");
    if (path_env == NULL){
        fprintf(stderr, "PATH environment variable not set.\n");
        exit(EXIT_FAILURE);
    }

    char *path_copy = strdup(path_env);
    char *path_token = strtok(path_copy, ":");

    while (path_token != NULL){
        char path_buffer[MAX_INPUT_LENGTH];
        snprintf(path_buffer, sizeof(path_buffer), "%s/%s", path_token, command);

        if (access(path_buffer, X_OK) == 0){
            free(path_copy);
            return 1;
        }

        path_token = strtok(NULL, ":");
    }

    free(path_copy);
    return 0; 

int main(void) {
    char input[MAX_INPUT_LENGTH];

    while (1){
        printf("simple_shell$ "); 

        if (fgets(input, sizeof(input), stdin) == NULL){
            printf("\n");
            break;
        }

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0){
            // Skip empty lines.
            continue;
        }

        char *command = input;

        if (!command_exists(command)){
            fprintf(stderr, "Command not found: %s\n", command);
            continue;
        }

        pid_t pid = fork();

        if (pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0) 
            char *args[] = {command, NULL};

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
                if (exit_status != 0) {
                    fprintf(stderr, "Command exited with status %d\n", exit_status);
                }
            }
            else if (WIFSIGNALED(status)){
                fprintf(stderr, "Command terminated by signal %d\n", WTERMSIG(status));
            }
        }
    }

    printf("Exiting simple_shell.\n");
    return 0;
}


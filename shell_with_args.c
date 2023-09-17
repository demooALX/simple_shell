#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_LENGTH 1024

/*
*main: where the main function is executed
*printf: display the prompt
*token: Tokenize the input to separate the command and arguments
*Return: string output to the screen
*/

int main(void)
{
    char input[MAX_INPUT_LENGTH];

    while (1){
        printf("simple_shell$ ");

        if (fgets(input, sizeof(input), stdin) == NULL){
            printf("\n");
            break;
        }
        
        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0){
            continue;
        }

        pid_t pid = fork();

        if (pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0){
            char *token = strtok(input, " ");
            char *args[MAX_INPUT_LENGTH];
            int i = 0;

            while (token != NULL){
                args[i++] = token;
                token = strtok(NULL, " ");
            }

            args[i] = NULL;

            if (execvp(args[0], args) == -1){
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

    printf("Exiting simple_shell.\n");
    return 0;
}


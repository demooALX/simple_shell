#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_LENGTH 1024
#define BUFFER_SIZE 1024

/*
*custom_getline: function that allows to get line
*command_exists: function that check for if path is present 
*main: where the main function is executed
*printf: display the prompt
*token: Tokenize the input to separate the command and arguments
*Return: string output to the screen
*/

char *custom_getline(void){
    static char buffer[BUFFER_SIZE];
    static size_t buffer_position = 0;
    static size_t buffer_size = 0;
    
    char *line = NULL;
    size_t line_length = 0;
    
    while (1){
        if (buffer_position >= buffer_size){
            ssize_t bytes_read = read(STDIN_FILENO, buffer, BUFFER_SIZE);
            if (bytes_read <= 0){
                return NULL;
            }
            buffer_size = (size_t)bytes_read;
            buffer_position = 0;
        }
        
        if (buffer[buffer_position] == '\n'){
            buffer[buffer_position] = '\0';
            buffer_position++;
            break;
        }
        
        if (line_length == 0){
            line = malloc(1);
            if (line == NULL){
                perror("malloc");
                exit(EXIT_FAILURE);
            }
        }
        else{
            char *new_line = realloc(line, line_length + 2);
            if (new_line == NULL){
                perror("realloc");
                free(line);
                exit(EXIT_FAILURE);
            }
            line = new_line;
        }
        
        line[line_length++] = buffer[buffer_position++];
    }
    
    line[line_length] = '\0';
    return line;
}

int main(void){
    while (1){
        printf("simple_shell$ ");

        char *input = custom_getline();

        if (input == NULL){
            printf("\n");
            break;
        }

        if (strlen(input) == 0){
            free(input);
            continue;
        }

        pid_t pid = fork();

        if (pid == -1){
            perror("fork");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0){
            char *command = input;
            char *args[] = {command, NULL};

            if (execvp(command, args) == -1){
                perror("execvp");
                free(input);
                exit(EXIT_FAILURE);
            }
        }
        else{
            int status;
            free(input);
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


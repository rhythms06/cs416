#include "Shell.h"
#include <zconf.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
/* A custom shell.
 * Acceptable commands include:
 * [command options]
 * [command options] | [command options]
 * [command options] > [command options]
 * [command options] >> [command options]
 * cd
 * exit
*/

#define SIGINT 2

void handle_sigint(int signal) {
    printf("\n");
}

// i took this code from online so we might want to make subtle changes to it
char *trim(char *input)
{
	char *end;
	while(isspace((unsigned char) *input)) {
        input++;
    }
	if(*input == 0)
		return input;
	end = input + strlen(input) - 1;
	while(end > input && isspace((unsigned char) *end)) end--;
	*(end+1) = 0;
	return input;
}


char** tokenize_input(char* input, char* delimiters, char** tokens) {
    char* current_token;
    tokens = calloc(256, sizeof(char*));
    int i = 0;
    current_token = strtok(input, delimiters);
    while(current_token != NULL) {
        tokens[i] = current_token;
        i++;
        current_token = strtok(NULL, delimiters);
    }
    tokens[i] = NULL;
    return tokens;
}

void exec_comm(char** command) {
    // printf("Command: %s\n", command);
    // printf("Input: %s</end>\n", input);
    //TODO: Tokenize the user input!
    int status;

    // printtokens(tokens);
    if (fork() == 0) { // If I am a child
        if(execvp(command[0], command) < 0) {
            printf("%s \033[0;31mis not a command\e[0m\n", command[0]);
        }
        exit(0);
    } else { // I am the parent!!
        wait(NULL);
    }
} 

void exec_pipe(char** left, char** right) {
    // 0 is read end, 1 is write end 
    int pipefd[2];  
    pid_t p1, p2; 
  
    if (pipe(pipefd) < 0) { 
        printf("\nPipe could not be initialized"); 
        return; 
    } 
    p1 = fork(); 
    if (p1 < 0) { 
        printf("\nCould not fork"); 
        return; 
    } 
  
    if (p1 == 0) { 
        // Child 1 executing.. 
        // It only needs to write at the write end 
        close(pipefd[0]); 
        dup2(pipefd[1], STDOUT_FILENO); 
        close(pipefd[1]); 
  
        if (execvp(left[0], left) < 0) { 
            printf("\nCould not execute command 1.."); 
            exit(0); 
        } 
    } else { 
        // Parent executing 
        p2 = fork(); 
  
        if (p2 < 0) { 
            printf("\nCould not fork"); 
            return; 
        } 
  
        // Child 2 executing.. 
        // It only needs to read at the read end 
        if (p2 == 0) { 
            close(pipefd[1]); 
            dup2(pipefd[0], STDIN_FILENO); 
            close(pipefd[0]); 
            if (execvp(right[0], right) < 0) { 
                printf("\nCould not execute command 2.."); 
                exit(0); 
            } 
        } else { 
            // parent executing, waiting for two children 
            close(pipefd[1]);
            wait(NULL); 
            wait(NULL); 
        } 
    } 
}

int main() {

    signal(SIGINT, handle_sigint);

    // input holds the next line read from stdin
    char input[256] = "";
    // exit is true when the user executes "exit"
    bool exit = false;
    // While the user hasn't hit "exit", execute their wishes!
    while (exit == false) {
        // Print the current working directory, cwd[i]
        // Note: A default directory is printed if the current pathname is more than i characters long.
        char cwd[256] = "Current/Pathname/Is/Too/Long";
        getcwd(cwd, 256);
        printf("\033[0;36m%s \033[0;33m$ \e[0m", cwd);
        // Retrieve input.
        fgets(input, 256, stdin);
        // Tokenize input into commands[i][j] with { ; \n } delimiters.
        // Note: There can be at most i commands, and each command can be at most j characters long.
        char** commands = NULL;
        commands = tokenize_input(input, ";\n", commands);
        if (commands[0] == NULL) {
            // The user entered nothing. Restart loop.
            continue;
        }
        // Iterate over commands
        int i = 0;
        while (commands[i] != NULL) {
            if (strcmp(commands[i], "exit") != 0) {
                // Try executing the command as a piping operation.
                // char* pipeLeft = strtok(commands[i], "|");
                // char* pipeRight = strtok(NULL, "|");
                char** pipes = NULL;
                pipes = tokenize_input(commands[i], "|", pipes);
                if (pipes[1] != NULL) {

                    // TODO: Pipe pipes.
                    printf("Piping...\n");
                    char** leftpipe = NULL;
                    char** rightpipe = NULL;
                    leftpipe = tokenize_input(pipes[0], " ", leftpipe);
                    rightpipe = tokenize_input(pipes[1], " ", rightpipe);
                    exec_pipe(leftpipe, rightpipe);
                    
                } else {
                    // Try executing the command as a redirection.
                    // Note: strtok can't tell the difference between > and >>.
                    char* redirectLeft = strtok(commands[i], ">");
                    char* redirectRight = strtok(NULL, ">");
                    if (redirectRight != NULL) {
                        if (commands[i][strlen(redirectLeft) + 1] == '>') {
                            char** appends = NULL;
                            appends = tokenize_input(commands[i], ">", appends);
                            // TODO: Append appends.
                            printf("Appending...\n");
                        } else {
                            char** writes = NULL;
                            writes = tokenize_input(commands[i], ">", writes);
                            // TODO: Write writes.
                            printf("Writing...\n");
                        }
                    } else {
                        // Try executing the command as either 'cd' or as an input of execvp.
                        char **tokens = NULL;
                        tokens = tokenize_input(commands[i], " ", tokens);
                        char* commandName = tokens[0];
                        if (strcmp(commandName, "cd") == 0) {
                            char* pathname = tokens[1];
                            if (tokens[1] == NULL) {
                                chdir("~");
                            }
                            if (tokens[1] != NULL) {
                                pathname = trim(pathname); // trims whitespace
                                chdir(pathname);
                            }
                        } else {
                            exec_comm(tokens);
                        }
                    }
                }
                i++;
            } else {
                // The current command is "exit."
                exit = true;
                break;
            }
        }
    }
    return 0;
}
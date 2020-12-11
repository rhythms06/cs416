#include "Shell.h"
#include <zconf.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
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

int input_length(char **input)
{
	int size = 0;

	while(input[size] != NULL) 
		size++;
	return size;
}

int find_redir(char** tokens) {
    int i = 0;

    while (tokens[i] != NULL) {
        if (strcmp(tokens[i], ">>") ) {
            return 2;
        } else if (strcmp(tokens[i], ">")) {
            return 1;
        }
        i++;
    }

    return 0;
}

char *trim(char *input) {
	char *end;
	while(isspace((unsigned char) *input)) {
        input++;
    }
	if (*input == 0) {
		return input;
    }
	end = input + strlen(input) - 1;
	while(end > input && isspace((unsigned char) *end)) {
        end--;
    }
	*(end + 1) = 0;
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


int exec_pipe(char **commands) {
	int curin = dup(0);	// Get current standard in and out
	int curout = dup(1);			
	
    const int STANDARD_IN = 0;
    const int STANDARD_OUT = 1;

	int input_file_descriptor = 0;
    int output_file_descriptor;
	if(!input_file_descriptor) { // set input fd
		input_file_descriptor = dup(curin);
    }

    int i = 0;
	for(i = 0; i < input_length(commands); i++) {
        char** tokens = NULL;
		tokens = tokenize_input(commands[i], " ", tokens); // Split command args by spaces
		dup2(input_file_descriptor, STANDARD_IN); // set to stdin
		close(input_file_descriptor); // close it? idk
        int redir = find_redir(tokens);

		if(i == input_length(commands) - 1) { // if we're at the end of the shit
			output_file_descriptor = dup(curout); // we out
        }
		else { // if we're not at the end 
			int file_descriptors[2]; // make an array of file descriptors

			pipe(file_descriptors); // pass in fds to array
			output_file_descriptor = file_descriptors[1]; // yup we piped
			input_file_descriptor = file_descriptors[0]; // we're piping
		}	
		dup2(output_file_descriptor, STANDARD_OUT); // some dup2 magic, i think this is what directs the output


		close(output_file_descriptor);
		
		// ezpz  forking and stuff
		if(fork() == 0) {
			execvp(tokens[0], tokens);
			exit(EXIT_FAILURE);
		} else {
            wait(NULL);
        }
		
	}
	dup2(curin, STANDARD_IN); // put this stuff back 
	dup2(curout, STANDARD_OUT);
	close(curin);
	close(curout);

	return 1;
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

                    //leftpipe = tokenize_input(pipes[0], " ", leftpipe);
                    //rightpipe = tokenize_input(pipes[1], " ", rightpipe);
                    //exec_pipe(leftpipe, rightpipe);
                    exec_pipe(pipes);
                    
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
                                // No pathname given. Navigate HOME.
                                chdir(getenv("HOME"));
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
                // The current command is "exit"
                exit = true;
                break;
            }
        }
    }
    return 0;
}
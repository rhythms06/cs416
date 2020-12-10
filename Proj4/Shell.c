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


char** tokenize_input(char* input, char** tokens) {
    char* current_token;
    tokens = calloc(256, sizeof(char*));
    int i = 0;
    current_token = strtok(input, " ");
    while(current_token != NULL) {
        tokens[i] = current_token;
        i++;
        current_token = strtok(NULL, " ");
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
        char commands[256][256];
        char* command = strtok(input, ";\n");
        if (command == NULL) {
            continue;
        }
        strcpy(commands[0], command);
        command = strtok(NULL, ";\n");
        int i = 0;
        while (command != NULL) {
            strcpy(commands[i + 1], command);
            i++;
            command = strtok(NULL, ";\n");
        }
        // Iterate over commands
        for (int j = 0; j <= i; j++) {
            if (strcmp(commands[j], "exit") != 0) {
                // Try executing the command as a piping operation.
                char* pipeLeft = strtok(commands[j], "|");
                char* pipeRight = strtok(NULL, "|");
                if (pipeRight != NULL) {
                    // TODO: Pipe pipeLeft into pipeRight
                    printf("You want to pipe the output of: \n");
                    printf("%s\n", pipeLeft);
                    printf("into the input of: \n");
                    printf("%s\n", pipeRight);
                } else {
                    // Try executing the command as a redirection.
                    // Note: strtok can't tell the difference between > and >>.
                    char* redirectLeft = strtok(commands[j], ">");
                    char* redirectRight = strtok(NULL, ">");
                    if (redirectRight != NULL) {
                        // TODO: Redirect the output of redirectLeft into redirectRight
                        printf("You want to redirect the output of: \n");
                        printf("%s\n", redirectLeft);
                        printf("into the file: \n");
                        printf("%s\n", redirectRight);
                    } else {
                        // Try executing the command as either 'cd' or as an input of execvp.
                        char** tokenized_com;
                        tokenized_com = tokenize_input(commands[j], tokenized_com);

                        char* commandName = tokenized_com[0];
                        if (strcmp(commandName, "cd") == 0) {
                            // TODO: Use chdir to execute cd command
                            char* pathname = tokenized_com[1];
                            pathname = trim(pathname); // trims whitespace
                            chdir(pathname);
                        } else {
                            // TODO: Use execvp to execute miscellaneous command
                            // char* inputs = strtok(NULL, "");
                            // if (inputs != NULL)
                            //     inputs = trim(inputs);
                            exec_comm( tokenized_com );
                        }
                    }
                }
            } else {
                // The current command is "exit."
                exit = true;
                break;
            }
        }
    }
    return 0;
}
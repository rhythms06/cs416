#include "Shell.h"
#include <zconf.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdbool.h>
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
    printf("Signal caught");
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
        printf("%s $ ", cwd);
        // Retrieve input.
        fgets(input, 256, stdin);
        // Tokenize input into commands[i][j] with { ; \n } delimiters.
        // Note: There can be at most i commands, and each command can be at most j characters long.
        char commands[256][256];
        char* command = strtok(input, ";\n");
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
                        char* commandName = strtok(commands[j], " ");
                        if (strcmp(commandName, "cd") == 0) {
                            // TODO: Use chdir to execute cd command
                        } else {
                            // TODO: Use execvp to execute miscellaneous command
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
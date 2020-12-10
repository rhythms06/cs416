#include "Shell.h"
#include <zconf.h>
#include <printf.h>
#include <string.h>
#include <stdbool.h>

int main() {
    // input holds the next line read from stdin
    char input[256] = "";
    // exit is true when the user executes "exit"
    bool exit = false;
    // While the user hasn't hit "exit", execute their wishes!
    while (exit == false) {
        // Print the current working directory
        char cwd[256] = "Current/Pathname/Is/Too/Long";
        getcwd(cwd, 256);
        printf("%s $ ", cwd);
        // Retrieve input
        fgets(input, 256, stdin);
        /*
         * commands = strtok \n and ;
         * for token in commands:
         *  if token is pipe { pipes... }
         *  else if token is redirection { dup2... }
         *  else execvp( strtok " " )
         */
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
        // Iterate through the inputted commands
        for (int j = 0; j <= i; j++) {
            if (strcmp(commands[j], "exit") != 0) {
                printf("TODO: Execute \"%s\"\n", commands[j]);
            } else {
                exit = true;
                break;
            }
        }
    }
    return 0;
}
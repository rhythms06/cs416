#include "Shell.h"
#include <zconf.h>
#include <printf.h>
#include <string.h>
#include <stdbool.h>

int main() {
    // cmd holds the next line read from stdin
    char cmd[256] = "";
    // exit is true when the user executes "exit"
    bool exit = false;
    // While the user hasn't hit "exit", execute their wishes!
    while (exit == false) {
        // Print the current working directory
        char cwd[256] = "CWD";
        getcwd(cwd, 256);
        printf("%s $ ", cwd);
        // Retrieve and tokenize cmd
        fgets(cmd, 256, stdin);
        char* token = strtok(strtok(cmd, "\n"), " ");
        if (strcmp(token, "exit") != 0) {
            while (token != NULL) {
                printf("You entered: %s", token);
                token = strtok(NULL, " ");
                printf("\n");
            }
        } else {
            exit = true;
        }
    }
    return 0;
}
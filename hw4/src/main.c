#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <readline/readline.h>

#include "sfish.h"
#include "debug.h"

int main(int argc, char *argv[], char* envp[]) {
    char* input;
    bool exited = false;

    Sigemptyset(&mask);
    Sigaddset(&mask, SIGINT);
    Sigaddset(&mask, SIGCHLD);
    Sigaddset(&mask, SIGTSTP);
    signal_main();
    signal(SIGTTOU, SIG_IGN);

    tcsetpgrp(STDIN_FILENO, tcgetpgrp(STDIN_FILENO));

    if(!isatty(STDIN_FILENO)) {
        // If your shell is reading from a piped file
        // Don't have readline write anything to that file.
        // Such as the prompt or "user input"
        if((rl_outstream = fopen("/dev/null", "w")) == NULL){
            perror("Failed trying to open DEVNULL");
            exit(EXIT_FAILURE);
        }
    }

    do {
        char* prompt = getprompt();
        input = readline(prompt);
        free(prompt);

        // If EOF is read (aka ^D) readline returns NULL
        if(input == NULL) {
            continue;
        }
        char* input_copy = malloc(strlen(input)+1);                 // need to free
        strcpy(input_copy, input);
        char *redirect_symbols = get_redirect_symbols(input_copy);  // need to free
        char **input_tokens = tokenize_input(input_copy, 0);        // need to free
        if (input_tokens[0] == NULL) {
            continue;
        }

        if (has_redirect(input) == 0) {                             //no redirect symbols
            if (builtins(input_tokens, 0) == 0) {                   //not a builtin
                executables(input_tokens, 0, NULL);                 //just attempt execute
            }
        } else {                                                    //has redirect symbols
            builtins(input_tokens, 1);
            char* r_input_copy = malloc(strlen(input)+1);
            strcpy(r_input_copy, input);
            char **r_input_tokens = tokenize_input(r_input_copy, 1);

            redirect(r_input_tokens, redirect_symbols);
            free(r_input_copy);
        }

        free(redirect_symbols);
        free(input_copy);
        free(input_tokens);

        // You should change exit to a "builtin" for your hw.
        exited = strcmp(input, "exit") == 0;

        // Readline mallocs the space for input. You must free it.
        rl_free(input);

    } while(!exited);

    debug("%s", "user entered 'exit'");

    return EXIT_SUCCESS;
}

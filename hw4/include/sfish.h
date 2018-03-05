#ifndef SFISH_H
#define SFISH_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <debug.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>
#include "sighandler.h"

/* Format Strings */
#define EXEC_NOT_FOUND "sfish: %s: command not found\n"
#define JOBS_LIST_ITEM "[%d] %s\n"
#define STRFTIME_RPRMT "%a %b %e, %I:%M%p"
#define BUILTIN_ERROR  "sfish builtin error: %s\n"
#define SYNTAX_ERROR   "sfish syntax error: %s\n"
#define EXEC_ERROR     "sfish exec error: %s\n"

/* My Functions */
int builtins(char** input_tokens, int has_redirect);
char* getprompt();
char **tokenize_input(char* input, int type);
void executables(char** input_tokens, int i, int* pipes);
int has_redirect(char* input);
char *get_redirect_symbols(char* input);
void redirect(char** r_input_tokens, char* redirect_symbols);

#endif

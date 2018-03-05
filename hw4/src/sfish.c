#include "sfish.h"

int builtins(char** input_tokens, int has_redirect) {
    if (strcmp(input_tokens[0], "exit") == 0) {
        // free(redirect_symbols);
        // free(input_copy);
        // free(input_tokens);
        exit(EXIT_SUCCESS);
    } else if ((strcmp(input_tokens[0], "help") == 0)  && (input_tokens[1] == NULL)) {
        if (has_redirect == 1) {
            return 2;
        } else {
            fprintf(stdout, "CSE320 HW3: Shell builtins include- help, cd, pwd, exit.\n");
            return 2;
        }
    } else if ((strcmp(input_tokens[0], "kill") == 0)  && (input_tokens[1] != NULL)) {
        debug("kill\n");
        debug("%s", input_tokens[1]);
        if (strcmp(input_tokens[1], "0") == 0) {
            fprintf(stdout, BUILTIN_ERROR, "invalid kill args");
            return 0;
        } else {
            int temp_pid = atoi(input_tokens[1]);
            if (temp_pid == 0) {
                fprintf(stdout, BUILTIN_ERROR, "invalid kill args");
                return 0;
            }
            debug("temp pid- %i\n", temp_pid);
            if (kill(temp_pid, SIGKILL) == -1) {
                fprintf(stdout, BUILTIN_ERROR, "invalid kill args");
            }
            return 1;
        }
    } else if ((strcmp(input_tokens[0], "cd") == 0)) {
        if ((input_tokens[1] == NULL)) {
            fprintf(stdout, "cd: to home dir\n");
            char* buf = NULL;
            buf = getcwd(buf, 0);
            setenv("OLDPWD", buf, 1);
            free(buf);
            chdir(getenv("HOME"));
            return 1;
        } else if (strcmp(input_tokens[1], "-") == 0) {
            char* old_dir = getenv("OLDPWD");
            if (old_dir == NULL) {
                fprintf(stdout, "cd -: OLDPWD not set\n");
            } else {
                fprintf(stdout, "cd to previous dir\n");
                char* buf = NULL;
                buf = getcwd(buf, 0);
                setenv("OLDPWD", buf, 1);
                free(buf);
                chdir(old_dir);
            }
            return 1;
        } else if (strncmp(input_tokens[1], "..", 2) == 0) {
            fprintf(stdout, "cd: to parent subdirectory\n");
            char* buf = NULL;
            buf = getcwd(buf, 0);
            setenv("OLDPWD", buf, 1);
            if (chdir(input_tokens[1])) {
                fprintf(stdout, BUILTIN_ERROR, "No such file or directory");
            }
            free(buf);
            return 1;
        } else if (strcmp(input_tokens[1], ".") == 0) {
            fprintf(stdout, "cd: to same dir\n");
            char* buf = NULL;
            buf = getcwd(buf, 0);
            setenv("OLDPWD", buf, 1);
            free(buf);
            return 1;
        } else {
            // if (input_tokens[2] != NULL) {
            //     fprintf(stdout, SYNTAX_ERROR, "Too many args");
            //     return 1;
            // }
            char* buf = NULL;
            buf = getcwd(buf, 0);
            setenv("OLDPWD", buf, 1);
            if(chdir(input_tokens[1]) == -1) {
                fprintf(stdout, "%s: No such file or directory\n", input_tokens[1]);
            } else {
                fprintf(stdout, "cd to new directory\n");
            }
            free(buf);
            return 1;
        }
    } else if (strcmp(input_tokens[0], "pwd") == 0) {
        if (has_redirect == 1) {
            return 3;
        } else {
            char* buf = NULL;
            buf = getcwd(buf, 0);
            fprintf(stdout, "Current working directory is: %s\n", buf);
            free(buf);
            return 3;
        }
    } else {
        return 0;
    }
}

char* getprompt() {
    char* pwd_buffer = NULL;
    pwd_buffer = getcwd(pwd_buffer, 0);
    char* prompt = calloc(1, strlen(pwd_buffer)+12);
    char* home = getenv("HOME");
    size_t n = strlen(home);
    if (strcmp(home, pwd_buffer) == 0) {
        strcat(prompt, pwd_buffer);
        strcat(prompt, ":: cjli >> ");
    }
    else if (strncmp(home, pwd_buffer, n) == 0) {
        strcat(prompt, "~");
        strcat(prompt, pwd_buffer+n);
        strcat(prompt, " :: cjli >> ");
    }
    else {
        strcat(pwd_buffer, ":: cjli >> ");
    }

    free(pwd_buffer);
    return prompt;
}


char **tokenize_input(char* input, int type) {
    char* delim;
    if (type == 0) {
        delim = " \t";
    } else if (type == 1) {
        delim = "<>|";
    }
    char** input_tokens = malloc(sizeof(char*)*strlen(input));
    char* token = strtok(input, delim);

    int n = 0;
    while(token != NULL) {
        input_tokens[n] = token;
        token = strtok(NULL, delim);
        n++;
    }
    input_tokens[n++] = NULL;
    return input_tokens;
}

void executables(char** input_tokens, int i, int* pipes) {
    Sigprocmask(SIG_BLOCK, &mask, &prev_mask);
    // Sigprocmask(SIG_SETMASK, &prev_mask, NULL);

    pid_t origpid = tcgetpgrp(STDIN_FILENO);
    pid_t pid = fork();
    if (pid == 0) {         //In child process
        signal(SIGTTOU, SIG_IGN);
        pid_t ch_pid = getpid();
        setpgid(ch_pid, ch_pid);
        tcsetpgrp(STDIN_FILENO, ch_pid);
        if (input_tokens[i] == NULL) {
            fprintf(stdout, EXEC_ERROR, "Nothing to execute.");
            exit(EXIT_SUCCESS);
        }
        if (strcmp(input_tokens[i], "help") == 0) {
            fprintf(stdout, "CSE320 HW3: Shell builtins include- help, cd, pwd, exit.\n");
            exit(EXIT_SUCCESS);
        }
        if (strcmp(input_tokens[i],"pwd") == 0) {
            char* buf = NULL;
            buf = getcwd(buf, 0);
            fprintf(stdout, "Current working directory is: %s\n", buf);
            free(buf);
            exit(EXIT_SUCCESS);
        }
        if(pipes != NULL) {
            close(pipes[0]);
            close(pipes[1]);
        }
        if(execvp(input_tokens[i], input_tokens) == -1) {
            fprintf(stdout, EXEC_ERROR, "Executable not found.");
            exit(EXIT_SUCCESS);
        }
    } else {    // in parent
        setpgid(pid, pid);
        tcsetpgrp(STDIN_FILENO, pid);

        sigsuspend(&prev_mask);
        Sigprocmask(SIG_UNBLOCK, &mask, &prev_mask);
        tcsetpgrp(STDIN_FILENO, origpid);
    }
    return;
}

int has_redirect(char* input) { //return 1 if has redirect symbol. else return 0
    if (strchr(input, '<') != NULL) {
        return 1;
    } else if (strchr(input, '>') != NULL) {
        return 1;
    } else if (strchr(input, '|') != NULL) {
        return 1;
    } else {
        return 0;
    }
}

char *get_redirect_symbols(char* input) {
    char* redirect_symbols = malloc(sizeof(char*)*strlen(input));
    int r = 0;
    int n = 0;
    while(*(input+n) != '\0') {
        if (*(input+n) == '<' || *(input+n) == '>' || *(input+n) == '|') {
            *(redirect_symbols+r) = *(input+n);
            r++;
        }
        n++;
    }
    *(redirect_symbols+r) = '\0';
    return redirect_symbols;
}

void redirect(char** strings, char* symbols) {
    // what if length of (r input tokens) != length of (redirectsymbols + 1)?
    // it should follow format of [operation] [symbol] [destination/operation] ... ?
    // if not, invalid input? will it crash? ill test it later i guess
    int orig_stdin = (dup(STDIN_FILENO));
    int orig_stdout = (dup(STDOUT_FILENO));
    int left_exec = 0;
    int sym_c = 0;
    char last_symbol = symbols[sym_c];
    while (symbols[sym_c] != '\0') {
        if (symbols[sym_c] == '<') {          // handle <
            //tokenize strings to get rid of whitespace
            char **string_tokens = tokenize_input(strings[sym_c+1], 0);     // need to free
            int in_fd = open(string_tokens[0], O_RDONLY);
            free(string_tokens);
            if (in_fd < 0) {
                fprintf(stdout, SYNTAX_ERROR, strerror(errno));
                return;
            }
            dup2(in_fd, STDIN_FILENO);
        } else if (symbols[sym_c] == '>') {   // handle >
            char **string_tokens = tokenize_input(strings[sym_c+1], 0);     // need to free
            int out_fd = open(string_tokens[0], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
            free(string_tokens);
            if (out_fd < 0) {
                fprintf(stdout, SYNTAX_ERROR, strerror(errno));
                return;
            }
            dup2(out_fd, STDOUT_FILENO);
        } else {     // handle | (pipe)
            int pipes[2];
            if (pipe(pipes) < 0) {
                fprintf(stdout, SYNTAX_ERROR, strerror(errno));
                return;
            }
            dup2(pipes[1], STDOUT_FILENO);
            char **left_tokens = tokenize_input(strings[left_exec], 0);     // need to free
            executables(left_tokens, 0, pipes);
            dup2(pipes[0], STDIN_FILENO);
            close(pipes[0]);
            close(pipes[1]);
            free(left_tokens);
            //update leftmost
            left_exec = sym_c+1;
        }
        last_symbol = symbols[sym_c];
        sym_c++;
    }
    //after loop, exec one last time, on the leftmost token
    char **exec_tokens = tokenize_input(strings[left_exec], 0);     // need to free
    if (last_symbol == '>') {
        //execute, then restore output, then restore input
        executables(exec_tokens, 0, NULL);
        dup2(orig_stdout, STDOUT_FILENO);
        dup2(orig_stdin, STDIN_FILENO);
    } else if (last_symbol == '|') {
        //restore output
        dup2(orig_stdout, STDOUT_FILENO);
        //then, if no input given, execute first
        if (exec_tokens[1] == NULL) {
            executables(exec_tokens, 0, NULL);
            dup2(orig_stdin, STDIN_FILENO);
        } else {    //otherwise, restore stdin first
            dup2(orig_stdin, STDIN_FILENO);
            executables(exec_tokens, 0, NULL);
        }
    } else {
        executables(exec_tokens, 0, NULL);
        dup2(orig_stdout, STDOUT_FILENO);
        dup2(orig_stdin, STDIN_FILENO);
    }

    free(exec_tokens);
    return;
}

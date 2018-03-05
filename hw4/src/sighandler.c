#include "sighandler.h"

#include <sys/wait.h>
#include <errno.h>
#include <stdio.h>
#include <debug.h>

void siginthandler(int signum) {
debug("sig int handler");
// idk wat do
}

void sigchldhandler(int signum) {
debug("sig child handler");
    int status;
    waitpid(-1, &status, (WUNTRACED | WNOHANG));
    //check status for WIFSTOPPED, if it is, add to job list
    // if (WIFSTOPPED(status) != 0) {
    //     //add to job list
    //     joblist_a[list_counter].jid = list_counter;
    //     list_counter++;
    //     joblist_a[list_counter].exec_name = input_tokens[0];
    // }
}

void sigtstphandler(int signum) {
//default action - suspend
debug("sig stop handler");
}

handler_t* Signal(int signum, handler_t handler){
    struct sigaction action, old_action;

    action.sa_handler = handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_RESTART | SA_SIGINFO;

    if(sigaction(signum, &action, &old_action) < 0)
        printf("Signal error: .............");

    return(old_action.sa_handler);
}

void signal_main() {
    Signal(SIGINT, siginthandler);
    Signal(SIGCHLD, sigchldhandler);
    Signal(SIGTSTP, sigtstphandler);
}

void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset) {
    if (sigprocmask(how, set, oldset) < 0)
        printf("Sigprocmask error");
    return;
}

void Sigemptyset(sigset_t *set) {
    if (sigemptyset(set) < 0)
        printf("Sigemptyset error");
    return;
}

void Sigaddset(sigset_t *set, int signum) {
    if (sigaddset(set, signum) < 0)
        printf("Sigaddset error");
    return;
}


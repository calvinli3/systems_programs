#ifndef SIGHANDLER_H
#define SIGHANDLER_H

#include <signal.h>

sigset_t mask, prev_mask;
typedef void (handler_t)(int);

void siginthandler(int signum);
void sigchldhandler(int signum);
void sigtstphandler(int signum);

handler_t* Signal(int signum, handler_t *handler);
void signal_main();

void Sigemptyset(sigset_t *set);
void Sigaddset(sigset_t *set, int signum);
void Sigprocmask(int how, const sigset_t *set, sigset_t *oldset);

#endif

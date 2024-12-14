#ifndef _SIGNA_HANDLERS_H_
#define _SIGNA_HANDLERS_H_

void sigchld_handler(int signo);
void sigtstp_handler();
void sigint_handler();

#endif
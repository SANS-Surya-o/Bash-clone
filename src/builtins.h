#ifndef BUILTINS_H
#define BUILTINS_H
#include "shell.h"


extern char* builtin_cmds[];
extern int num_builtin_cmds;

typedef int (*builtin_func)(cmd* command);
extern builtin_func builtin_funcs[];

int hop(cmd* command);
int reveal(cmd* command);
int proclore(cmd* command);
int seek(cmd* command);
int activities(cmd* command);
int ping(cmd* command);
int fg(cmd* command);
int bg(cmd * command);
int iMan(cmd* command);
int neonate(cmd* command);

#endif




#ifndef LOG_H
#define LOG_H
#include "shell.h"

int log_init();
int log_save(char* line);
int log_b(cmd* command);
int log_save_to_file();

#endif


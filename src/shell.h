#ifndef SHELL_H
#define SHELL_H
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/wait.h>
#include <dirent.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/select.h>
#include <termios.h>
#include <aio.h>
// #include "data_structs.h"
#include "colours.h"
#include "directory.h"
#include "string_utils.h"
#include <libgen.h>

extern pthread_mutex_t process_list_mutex;

#define DEBUGGING_MODE 0
#define FOREGROUND_PROC 0
#define BACKGROUND_PROC 1
#define REPLACE_ALIAS 1
#define DONT_REPLACE_ALIAS 0


#define CATASTROPHE -69 

#define PATH_MAX 4096

extern char* username;
extern char system_name[1024];
extern char ENV_FILE[1024];
extern pid_t shell_pgid;
extern pid_t fg_pgid;
extern int checker;

extern char myshrc_file_location[1024];

// extern char* log_file_location;
char* get_directory();


// What is the maximum number of commands which can be piped : ax number of commands separated by & or ;
#define MAX_CMD_LEN 1024
// #define MAX_ARGS 64
#define MAX_COMMANDS 100





typedef struct cmd{
    char* command;
    char** tokens;
    int background;
}cmd;




cmd* create_cmd(char* tokens, int background);
void print_cmd(cmd** command);
void destroy_cmd(cmd* command);
bool valid_cmd(cmd* command);
void memory_cleanup(cmd** commands);
void shut_down_shell();



void init_shell();


#endif




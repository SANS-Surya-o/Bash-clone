
#include "shell.h"
#include "input.h"
#include "execute.h"
#include "colours.h"
#include "builtins.h"
#include "log.h"
#include "processManagement.h"
#include "directory.h"
#include <signal.h>
#include "signal_handlers.h"
#include "raw_mode.h"
#include "alias.h"




char* username = NULL;        // Declared in shell.h. Defined in main.c
char system_name[1024];





int main()
{

    init_shell();
    log_init();
    // enable_raw_mode();
    atexit(shut_down_shell);

   
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP; // Restart interrupted system calls and don't catch stopped children
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror("sigaction");
        exit(1);
    }

    // Ignore both ctr-z and ctrl-c. When children are created for foreground processes, set their signal behaviour to default. When waiting for them, if they exited with a stopped status 
    // code performs necessary changes 

    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;  // Restart interrupted system calls
    if (sigaction(SIGTSTP, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    sa.sa_handler = SIG_IGN;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    while(1)
    {   
        directory_update();
        if (fg_pgid!=-1)
        {
            printf("Shell runs though fg_pgid not set");
            exit(1);
        }
       
            // printf(BLUE"pid : %d"RESET,getpid());
        
        if (recent_foreground_proc==NULL || recent_foreground_proc->time<=2)
            {printf(BOLDGREEN"%s@%s"BOLDBLUE":%s>"RESET,username,system_name,directory);}
        else
        {
            printf(BOLDGREEN"%s@%s"BOLDBLUE":%s %s:%ds>"RESET,username,system_name,directory,recent_foreground_proc->command,recent_foreground_proc->time);
        }

        fflush(stdout);
        // Obtain the cmd, exit if user wants to, store the command in log file
        cmd** commands = extract_cmds_from_input();
        // Error handling is done in manage_input


        // print the cmds and args for debugging
        if (DEBUGGING_MODE)
        {
           print_cmd(commands);
        }
        if (!alias_found)
        {
            fprintf(stderr,YELLOW"myshrc not found : alias clear disabled\n"RESET);
        }

        execute(commands);
       
    }
}
      
        
       
        






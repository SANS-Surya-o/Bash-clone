#include "shell.h"
#include "signal_handlers.h"
#include "processManagement.h"


void sigchld_handler(int sig) {
    int saved_errno = errno; 
    int status;
    pid_t pid;

    
 
    // Reap all dead children
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        process* p = find_process(pid);
        if (p==NULL) continue;
        //  print_all_processes();
        if (WIFEXITED(status)) {
            if (p!=NULL && p->command!=NULL)
            printf("%s exited normally (%d)\n", p->command, p->pid);

        } else if (WIFSIGNALED(status)) {
            if (p!=NULL && p->command!=NULL)
            printf("%s exited abnormally (%d)\n", p->command, p->pid);
           
        }
        else if (WIFSTOPPED(status))
        {   
            if (p!=NULL && p->command!=NULL)
            printf("%s stopped by a signal (%d)\n", p->command, p->pid);
        }
       
        delete_process(pid);
        fflush(stdout);
    }
    errno = saved_errno;
}



  
#include "shell.h"
#include "builtins.h"
#include "directory.h"
#include "processManagement.h"
#include "input.h"


int fg(cmd* command) {
    if (command == NULL || command->tokens == NULL || command->tokens[1] == NULL) {
        return -1;
    }

    char** tokens = command->tokens;
    int n = 0;
    while (tokens[n] != NULL) {
        n++;
    }

    if (n > 2) {
        fprintf(stderr,"fg: too many operands\n");
        return -1;
    }
    if (n < 2) {
        fprintf(stderr,"fg: missing operand\n");
        return -1;
    }
    if (!isnum(tokens[1])) {
        fprintf(stderr,RED "fg: invalid operand\n" RESET);
        return -1;
    }


    int pid = atoi(tokens[1]);
   
    process* p = find_process(pid);
    
    if (p==NULL)
    {
        if ( kill(pid, 0) == -1 )
        {
            perror("fg : ");
            return -1;
        }
        fprintf(stderr,"External process - Can't be brought to foreground as shell cant wait for processes which are not children. If stopped, it will be continued though\n"); 
        if (kill(pid, SIGCONT) < 0) {
            perror(RED "fg : Failed to continue process" RESET);
            return -1;
        }
    }
    else
    {
        
        // Process spawned by shell
        fg_pgid = pid;

        if (kill( -fg_pgid , 0) == -1)
        {
            fprintf(stderr, "Signal sent when signal handling going on : Ctrl-Z and process termiantion clashed. The current process termianted though not updated in process list. Now the process is cleared.\n");
            return -1;
        }

        if (tcsetpgrp(STDIN_FILENO, fg_pgid) < 0) {
            perror(RED "fg : tcsetpgrp failed" RESET);
            fg_pgid = -1;
            return -1;
        }

        char* proc_cmd = strdup(p->command);

        if (kill(-fg_pgid, SIGCONT) < 0) {
            perror("fg : Failed to continue process");
            fg_pgid = -1;
            return -1;
        }
        p->state = PROCESS_RUNNING;
        int status;

        struct timeval start, end;
        gettimeofday(&start, NULL);
        if (fg_pgid == -1) {
            return -1;
        }
        int stopped = 0;
        while(waitpid(-fg_pgid, &status, WUNTRACED) > 0) {
            if (WIFSTOPPED(status)) {
                p->state = PROCESS_STOPPED;
                fprintf(stderr,"Pipeline stopped (process group %d)\n", pid);
                stopped = 1;
                break;
            }
        }
        if (!stopped)
        {
            fprintf(stderr,GREEN"Process %d brought to foreground terminated\n"RESET, pid);
            delete_process(fg_pgid);
        }
        
        gettimeofday(&end, NULL);
        if (recent_foreground_proc!=NULL)
        {
            destroy_process(recent_foreground_proc);
        }

        recent_foreground_proc = create_process(fg_pgid, proc_cmd);
        recent_foreground_proc->time = (int)(end.tv_sec - start.tv_sec);


        fg_pgid = -1;
        free(proc_cmd);
        if (tcsetpgrp(STDIN_FILENO, shell_pgid) < 0) {
            perror("fg : tcsetpgrp failed");
            return -1;
        }
    }
    return 1;
}




   

   
    
   




int bg(cmd * command)
{
    if (command == NULL || command->tokens == NULL || command->tokens[1] == NULL) {
        return -1;
    }

    char** tokens = command->tokens;
    int n = 0;
    while (tokens[n] != NULL) {
        n++;
    }

    if (n > 2) {
        printf("bg: too many operands\n");
        return -1;
    }
    if (n < 2) {
        printf("bg: missing operand\n");
        return -1;
    }
    if (!isnum(tokens[1])) {
        printf(RED "bg: invalid operand\n" RESET);
        return -1;
    }

    int pid = atoi(tokens[1]);


    process* p = find_process(pid);



    // If process spawned by shell, send SIGCONT to the entire process group 
    // If external process, send SIGCONT only to the process
    if (p!=NULL)
    {   // Send kill to whole process grp
        if (kill(-pid , 0) == -1)
        {
            fprintf(stderr, "Signal sent when signal handling going on : Ctrl-Z and process termiantion clashed. The current process termianted though not updated in process list. Now the process is cleared.\n");
            return -1;
        }
        if (kill(-pid, SIGCONT) < 0) {
            perror("bg : Failed to continue process");
            return -1;
        }
        p->state = PROCESS_RUNNING;
    }
    else
    {
        if ( kill(pid,0) == -1 )
        {
            perror("bg");
            return -1;
        }

        printf("External process - won't show in activities\n");
        if (kill(pid,SIGCONT) < 0)
        {
            perror("bg : pid not of a proc spawned by shell:  Failed to continue process");
            return -1;
        }
    }
    
    return 1;
}








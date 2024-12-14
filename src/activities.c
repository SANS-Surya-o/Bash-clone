#include "shell.h"
#include "input.h"
#include "builtins.h"
#include "processManagement.h"


int cmp_activities(const void* p1 ,const void * p2)
{
    return ( (*(const process**)p1)->pid > (*(const process**)p2)->pid );
}


int activities(cmd* command)
{
    int i=0; 
    while(command->tokens[i]!=NULL)
    {
        i++;
    }
    if (i>1)
    {
        fprintf(stderr,RED"Activities : Activities accepts no arguments\n"RESET);
        return -1;
    }

    int nmemb = 0;
    process** procs = get_all_processes(&nmemb);
    qsort(procs,nmemb,sizeof(process*),cmp_activities);
    for (int i=0;i<nmemb;i++)
    {
        printf("%d : %s -",procs[i]->pid,procs[i]->command);
        if (procs[i]->state==PROCESS_RUNNING)
        {
            printf("Running\n");
        }
        else if (procs[i]->state==PROCESS_STOPPED)
        {
            printf("Stopped\n");
        }
    }
    free(procs);
    return 1;
}

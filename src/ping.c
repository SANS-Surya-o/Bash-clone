#include "shell.h"
#include "builtins.h"
#include "input.h"





int ping(cmd * command)
{
    if (command==NULL || command->tokens==NULL || command->tokens[0]==NULL)
    {
        return -1;
    }
    char** tokens = command->tokens;
    int n =0 ;
    while(tokens[n]!=NULL)
    {
        n++;
    }
    if (n<3)
    {
        printf("ping: missing operand\n");
        return -1;
    }
    if (n>3)
    {
        printf("ping: too many operands\n");
        return -1;
    }

    if (!isnum(tokens[1]) || !isnum(tokens[2]))
    {
        return -1;
    }

    int pid = atoi(tokens[1]);
    int sig = atoi(tokens[2]);

    sig = (sig % 32);

    if (kill(pid, 0) == -1) {
        perror("ping :");
        return -1;
    }

    if (kill(pid, sig) == -1) {
        perror("ping : Signal not sent");
        return -1;
    }
    
}


    

#include "shell.h"
#include "directory.h"
#include "input.h"
#include "signal_handlers.h"
#include "processManagement.h"

pid_t shell_pgid;
pid_t fg_pgid = -1;
struct termios shell_tmodes;

char myshrc_file_location[1024];

int shell_terminal = STDIN_FILENO;
int shell_is_interactive;
char ENV_FILE[1024];
int checker = 0;
// Initialization
void init_shell()
{
    username = getlogin();
    if (username==NULL)
    {
        perror("username obtain");
        exit(1);
    }
        
    if (gethostname(system_name,1024) < 0)
    {
        perror("obtaining hostname of machine");
        exit(1);
    };
 
    init_directory();
    strcpy(ENV_FILE,home_directory); 
    strcat(ENV_FILE,"/ENV.txt");

    strcpy(myshrc_file_location, home_directory);
    strcat(myshrc_file_location,"/myshrc.txt");

     FILE* file;  
    file = fopen(ENV_FILE, "w");
    if (file == NULL) {
        perror("directory.c : directory_update : Failed to open environment file");
        exit(EXIT_FAILURE);
    }
    fprintf(file,"%s\n",home_directory);
    fclose(file);
  

        // Loop until the shell is in the foreground
        while (tcgetpgrp(shell_terminal) != (shell_pgid = getpgrp())) {
            kill(-shell_pgid, SIGTTIN);
        }

        // Create a new process group for the shell
        shell_pgid = getpid();
        if (setpgid(shell_pgid, shell_pgid) < 0) {
            perror("Couldn't put the shell in its own process group");
            exit(1);
        }

        // Take control of the terminal
            tcsetpgrp(shell_terminal, shell_pgid);
            signal(SIGTTOU, SIG_IGN);
            signal(SIGTTIN, SIG_IGN);

        // Save the terminal attributes for shell
        tcgetattr(shell_terminal, &shell_tmodes);
}
    

    


char* get_directory()
{
    char* dircopy = strdup(directory);
    return dircopy;   
}




// Command struct generation and handling




// Does not alter the inputs string
// Rules for freeing:  
cmd* create_cmd(char* inputs, int background)
{
    cmd* command = (cmd*)malloc(sizeof(cmd));
    if (command==NULL)
    {
        perror("malloc: cmd");
        return NULL;
    }
    char * line_copy = strdup(inputs);
    command->command = line_copy;
    command->tokens = strip_whitespaces(line_copy, REPLACE_ALIAS);
    if (command->tokens==NULL)
    {
        perror("Allocation failure: command creation");
        free(command);
        return NULL;
    }
    command->background = background;
    return command;
}

void destroy_cmd(cmd* cmd)
{
    if (cmd==NULL) return;
    if (cmd->tokens!=NULL)
    {
        int i=0;
       while(cmd->tokens[i]!=NULL)
       {
              free(cmd->tokens[i]);
              cmd->tokens[i]=NULL;
              i++;
       }
        free(cmd->tokens);
        cmd->tokens=NULL;
    }
    if (cmd->command!=NULL)
    {
        free(cmd->command);
        cmd->command=NULL;
    }
    free(cmd);
    cmd=NULL;
}
       
       
        

void print_cmd(cmd** commands)
{
    if (commands==NULL)
    {
        printf("NULL\n");
        return;
    }
    int i=0;
    while(commands[i]!=NULL)
    {
        char** tokens = commands[i]->tokens;
        int j=0;
        while(tokens[j]!=NULL)
        {
            printf("%s ",tokens[j]);
            j++;
        }
        printf("---%d\n",commands[i]->background);
        i++;
    }
}

void memory_cleanup(cmd** commands)
{
    if (commands==NULL) return;
    int i=0;
    
    while(commands[i]!=NULL)
    {
        destroy_cmd(commands[i]);
        i++;
    }
    
    free(commands);
    commands = NULL;
}

void shut_down_shell()
{
    int pid = getpid();
    if (pid!=shell_pgid){
        return;
    }

    int nmemb = 0;
    process** procs = get_all_processes(&nmemb);
    for (int i=0;i<nmemb;i++)
    {
        kill(-(procs[i]->pid),SIGKILL);
    }
    free(procs);
}

bool valid_cmd(cmd* command)
{
    if (command == NULL || command->tokens == NULL || command->command == NULL || command->tokens[0] == NULL)
    {
        return false;
    }
    return true;
}
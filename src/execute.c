#include "shell.h"
#include "builtins.h"
#include "execute.h"
#include "log.h"
#include "colours.h"
#include "input.h"
#include "processManagement.h"



/*
    The execute module is long and ugly code that just works. 
    TO understand how it works (if you really want to), begin from the bottom and work your way up each successive function. Every function calls the function above it.
    The execute function is the entry point for the module.
*/


// If more built-in commands are to be added, add the string name here and the function pointer in the builtin_funcs array
char* builtin_cmds[] = {"hop","reveal","log","proclore","seek","activities","ping","fg","bg","neonate","iMan"};
int num_builtin_cmds = sizeof(builtin_cmds)/sizeof(char*);
builtin_func builtin_funcs[] = {
    &hop,
    &reveal,
    &log_b,
    &proclore,
    &seek,
    &activities,
    &ping,
    &fg,
    &bg,
    &neonate,
    &iMan
};

int built_in_cmd(cmd* command)    
{
    if (command==NULL || command->tokens==NULL || command->tokens[0]==NULL) return -1; 
    char** tokens = command->tokens;
    for(int i=0;i<num_builtin_cmds;i++)
    {
       if (strcmp(builtin_cmds[i],tokens[0])==0) 
       {
          return builtin_funcs[i](command);
       }
    }
    return 0;
}
            





// File rediection module///////////////////////////////////////////////////////////////////////
struct file_redirect
{
    char* input_file;
    char* output_file;
    int append;
};

// clean the command by removing the file src and file dest location from the command
void handle_redirection(char* input, char* cleaned_command, char* input_file, char* output_file, int* append) {
    int i = 0, j = 0;
    int in_redirect_found = 0, out_redirect_found = 0;

    // Initialize output variables to empty
    input_file[0] = '\0';
    output_file[0] = '\0';
    *append = 0;

    while (input[i] != '\0') {
        if (input[i] == '<' && !in_redirect_found) {
            // Skip the '<' and any following spaces
            i++;
            while (isspace(input[i])) i++;

            // Capture the input filename
            int k = 0;
            while (input[i] != '\0' && !isspace(input[i])) {
                input_file[k++] = input[i++];
            }
            input_file[k] = '\0';  // Null-terminate the input file
            in_redirect_found = 1;
        } else if (input[i] == '>') {
            // Check if it's '>>' for append
            if (input[i + 1] == '>') {
                *append = 1;  // Set append flag
                i += 2;       // Skip both '>' symbols
            } else {
                *append = 0;  // Simple '>'
                i++;          // Skip the '>' symbol
            }

            // Skip any following spaces
            while (isspace(input[i])) i++;

            // Capture the output filename
            int k = 0;
            while (input[i] != '\0' && !isspace(input[i])) {
                output_file[k++] = input[i++];
            }
            output_file[k] = '\0';  // Null-terminate the output file
            out_redirect_found = 1;
        } else {
            // Copy non-redirection parts of the command
            cleaned_command[j++] = input[i++];
        }
    }

    // Null-terminate the cleaned command
    cleaned_command[j] = '\0';
}


struct file_redirect* file_redirection(cmd* command)
{
    int n = strlen(command->command);
    char* input_file = (char*)malloc((n+2)*sizeof(char));
    char* output_file = (char*)malloc((n+2)*sizeof(char));
    char* cleaned_command = (char*)malloc((n+2)*sizeof(char));
    int append = 0;
    handle_redirection(command->command, cleaned_command, input_file, output_file, &append);
    int i = 0;
    while (command->tokens[i] != NULL) {
        free(command->tokens[i]);
        i++;
    }
    free(command->tokens);
    command->tokens = strip_whitespaces(cleaned_command,DONT_REPLACE_ALIAS);
    free(cleaned_command);
    struct file_redirect* fr = (struct file_redirect*)malloc(sizeof(struct file_redirect));
    fr->input_file = input_file;
    fr->output_file = output_file;
    fr->append = append;
    return fr;
}

          
int change_fd_for_file_redirection(int* in_fd, int* out_fd, struct file_redirect* fr)
{
        if (strlen(fr->input_file)>0) {
            int input_fd = open(fr->input_file, O_RDONLY);
            if (input_fd < 0) {
                perror("Error opening input file");
                free(fr->input_file);
                free(fr->output_file);
                free(fr);
                return -1;
            }
            (*in_fd) = input_fd;
        }
        if (strlen(fr->output_file)>0) {
            int output_fd;
            if (fr->append) {
                output_fd = open(fr->output_file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            } else {
                output_fd = open(fr->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            }
            if (output_fd < 0) {
                perror("Error opening output file");
                free(fr->input_file);
                free(fr->output_file);
                free(fr);
                return -1;
            }
            (*out_fd) = output_fd;
        }
        free(fr->input_file);
        free(fr->output_file);
        free(fr);
        return 1;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


pid_t execute_command(cmd* command, int in_fd, int out_fd, pid_t pgid, int is_foreground, int is_first_command) {
    
    // Log cant run in background... 
    if (command->command == NULL)
    {
        fprintf(stderr,"execute_command : Command->command is NULL\n");
        return -1;
    }
    if (command== NULL || command->tokens== NULL || command->tokens[0]==NULL)
    {
        return -1;
    }
    if (strcmp(command->tokens[0],"log")==0 || strcmp(command->tokens[0],"fg")==0 || strcmp(command->tokens[0],"bg")==0)
    {
        struct timeval start_time, end_time;
        gettimeofday(&start_time, NULL);
        // printf("log entered");
        int saved_in = dup(STDIN_FILENO);
        int saved_out = dup(STDOUT_FILENO);
        if (in_fd != STDIN_FILENO) {
            dup2(in_fd, STDIN_FILENO);  // Redirect input
            close(in_fd);
        }
        if (out_fd != STDOUT_FILENO) {
            dup2(out_fd, STDOUT_FILENO);  // Redirect output
            close(out_fd);
        }
        built_in_cmd(command);
        dup2(saved_in,STDIN_FILENO);
        dup2(saved_out,STDOUT_FILENO);
        close(saved_in);
        close(saved_out);
        gettimeofday(&end_time, NULL);

        if (strcmp(command->tokens[0],"log")==0)
        {
               if (recent_foreground_proc!=NULL)
        {
            destroy_process(recent_foreground_proc);
        }
        recent_foreground_proc = create_process(-1,command->command);
        recent_foreground_proc->time = (int)(end_time.tv_sec-start_time.tv_sec);
        }
     

        return CATASTROPHE;

    }


    pid_t pid = fork();
    
    if (pid < 0)
    {
        perror("execute_command : Fork\n");
        return -1;
    }
    else if (pid == 0) {  // Child process
        if (is_first_command) {
            pgid = getpid();  // Set the PGID to the PID of the first process
        }

        setpgid(0, pgid);  // Set the process group for this child

        if (is_foreground) {
            tcsetpgrp(STDIN_FILENO, pgid);  // Give terminal control to the foreground process group
            
            struct sigaction sa;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            sa.sa_handler = SIG_DFL;
            sigaction(SIGINT, &sa, NULL);  // Reset SIGINT to default
            sigaction(SIGTSTP, &sa, NULL);  // Reset SIGTSTP to default
            fg_pgid = pgid;
        }
        else
        {
            struct sigaction sa;
            sigemptyset(&sa.sa_mask);
            sa.sa_flags = 0;
            sa.sa_handler = SIG_DFL;
            sigaction(SIGTTIN, &sa, NULL);  // Reset SIGINT to default
            sigaction(SIGTTOU, &sa, NULL);
        }
        // For functions involving redirection, change the in_fd and out_fd accrodingly 
        struct file_redirect* fr = file_redirection(command);

        if (change_fd_for_file_redirection(&in_fd,&out_fd,fr)==-1)
        {
            exit(EXIT_FAILURE);
        }
        if (in_fd != STDIN_FILENO) {
            dup2(in_fd, STDIN_FILENO);  // Redirect input
            close(in_fd);
        }
        if (out_fd != STDOUT_FILENO) {
            dup2(out_fd, STDOUT_FILENO);  // Redirect output
            close(out_fd);
        }
        if (built_in_cmd(command)!=0)  // It is a built-in cmd 
        {
            if (is_foreground){   // canging state of terminal in a child process does not affect the parent
                 // Since even builtin cmds are run in child, save the state to a file 
                FILE* file = fopen(ENV_FILE, "w");
                if (file == NULL) {
                    perror("Failed to open environment file");
                    exit(1);
                }

                // Write the current directory to the file
                char cwd[PATH_MAX];
                if (getcwd(cwd, PATH_MAX) == NULL) {
                    perror("Failed to get current directory");
                    exit(1);
                }

                fprintf(file, "%s\n", cwd);
                if (prev_directory!=NULL)
                    {fprintf(file,"%s\n",prev_directory);}
                
                fclose(file);
            }
           
        }
        else    // Command was not a built-in command. 
        {
            if (execvp(command->tokens[0],command->tokens)==-1)
            {
                fprintf(stderr,RED"%s:command not found or wrong arguments\n"RESET,command->tokens[0]);
                exit(EXIT_FAILURE);  // Exit with a failure status
            }
            perror("execvp"); 
        }
        exit(EXIT_FAILURE);

    } 
    else if (pid>0)
    {
        if (is_first_command) {
            pgid = pid; 
        }
        if (is_foreground) {
            tcsetpgrp(STDIN_FILENO, pgid);  // Give terminal control to the foreground process group
        }
        setpgid(pid, pgid);  // Set the process group for the parent side as well
        return pgid;  
    }
}




#define PIPE_READ 0
#define PIPE_WRITE 1

int run_piped_cmds(cmd** commands, char* command_name,  int num_commands) {
    
    if (commands== NULL || command_name==NULL || !valid_cmd(commands[0])) {
        fprintf(stderr,"Execute: Syntax error near unexpected '|'\n");
        return -1;
    }
    if (command_name[strlen(command_name)-1]=='\n')
    {
        command_name[strlen(command_name)-1] = '\0';
    }

    // Check if process is foreground
    int is_foreground = (commands[0]->background == FOREGROUND_PROC);
    int i, in_fd = STDIN_FILENO;
    int out_fd;
    int pipe_fd[2];
    pid_t pgid = 0;
    int first_command = 1;

    for (i = 0; i < num_commands; i++) {
        if (i < num_commands - 1) {
            pipe(pipe_fd); 
        } else {
            pipe_fd[PIPE_WRITE] = STDOUT_FILENO;  // Last command outputs to stdout
        }
        out_fd = pipe_fd[PIPE_WRITE];  // Next command writes to this pipe (if no file redirection)

        // FIle redirection if any happens inside the execute command function itself
        pgid = execute_command(commands[i], in_fd, out_fd, pgid , is_foreground, first_command);
        if (is_foreground && pgid>1)
        {
            fg_pgid = pgid;
        }
       
        if (pgid > 1) 
        { first_command = 0; }   

     
        if (in_fd != STDIN_FILENO) {
            close(in_fd);
        }
        if (pipe_fd[PIPE_WRITE] != STDOUT_FILENO) {
            close(pipe_fd[PIPE_WRITE]);
        }

        in_fd = pipe_fd[PIPE_READ];  // Next command reads from this pipe
    }

    if (is_foreground) {
        // After launching all commands, wait for the entire process group

        // If pgid is still -1, either no child was created at all : only 
        if (fg_pgid == -1)
        {
            return -1;
        }

        struct timeval start_time, end_time;
        gettimeofday(&start_time, NULL);

        
        tcsetpgrp(STDIN_FILENO, fg_pgid);  // Set the pipeline process group as the foreground process group

        int status;
        pid_t pid;
        int x = 0;
        while ((pid = waitpid(-fg_pgid, &status, WUNTRACED)) > 0) {
            if (WIFSTOPPED(status)) {
                // Handle the entire process group being stopped
                    fprintf(stderr,"run_piped_cmds : Pipeline stopped (process group %d)\n", fg_pgid);
                    process* p = add_process(fg_pgid, command_name);
                    if (kill(-fg_pgid,0)<0)
                    {
                        printf("No process");
                        return -1;
                    }
                    if (p==NULL)
                    {
                        fprintf(stderr,"Process exited almost at the same time as Ctrl-Z : recent_foreground_process time not updated..");
                        return -1;
                    }
                    p->state = PROCESS_STOPPED;
                    break;
            } 
        }


        gettimeofday(&end_time, NULL);
        if (recent_foreground_proc!=NULL)
        {
            destroy_process(recent_foreground_proc);
        }
        recent_foreground_proc = create_process(-1,command_name);
        recent_foreground_proc->time = (int)(end_time.tv_sec-start_time.tv_sec);

    } else {
        // Process running in background
        fprintf(stderr,"%d\n", pgid);
        add_process(pgid, command_name);
    }
    return 1;
}
      






        


// As the name says, execute an entire pipeline
int execute_pipeline(cmd* command)
{
    if (!valid_cmd(command))
    {
        return -1;
    }

    // Extract the different commands of the same pipeline into piped_cmds and send them to execute_pipe_commands
    int piped_cmds_size = MAX_COMMANDS;
    cmd** piped_cmds = (cmd**)malloc(piped_cmds_size*sizeof(cmd*));
   
    for(int i=0;i<MAX_COMMANDS;i++)
    {
        piped_cmds[i] = NULL;
    }
  
    int position = 0;

    char* str = strdup(command->command);
    char* str_cpy_for_freeing = str;

    char* token;
    char* saveptr;
    int i = 0;
    for (;;str=NULL,i++)
    {
        token = strtok_r(str,"|",&saveptr);
      
        if (token==NULL)
        {
            break;
        }
        
        piped_cmds[position] = create_cmd(token,command->background);
        position++;
    }
    if (str_cpy_for_freeing!=NULL)
    {
        free(str_cpy_for_freeing);
    }
    str_cpy_for_freeing = NULL;
    str = NULL;
    
    for (int i=0;i<position;i++)
    {
        if (piped_cmds==NULL || !valid_cmd(piped_cmds[i])){
            fprintf(stderr,"Execute: Syntax error near unexpected '|'\n");
            memory_cleanup(piped_cmds);
            free(str_cpy_for_freeing);
            return -1;
        }

        if (strcmp(piped_cmds[i]->tokens[0],"exit")==0)
        {
            exit(EXIT_SUCCESS);
        }
    }

        
    char* command_name = strdup(command->command);
    int r = run_piped_cmds(piped_cmds, command_name, position);

    tcsetpgrp(STDIN_FILENO, shell_pgid);
    fg_pgid = -1;
    
    free(command_name);
    memory_cleanup(piped_cmds);
    return r;
}

 

//commands -> pipelines separated by & or ;
int execute(cmd** commands)
{

    if (commands==NULL) return -1;
    int i = 0;
    while(commands[i]!=NULL)
    {
        execute_pipeline(commands[i]);
        i++;
    }
    memory_cleanup(commands);
    return 1;
}
        













    
       

   

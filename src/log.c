#include "shell.h"
#include "log.h"
#include "input.h"
#include "execute.h"


char* log_cmds[15]; // Array to store the commands
char log_file_location[PATH_MAX];
int log_size = 0; 
char* log_file_name = "log.txt";






int log_init()
{
    snprintf(log_file_location, PATH_MAX , "%s/%s", home_directory,log_file_name);
    for (int i=0;i<15;i++)
    {
        log_cmds[i] = NULL;
    }

    FILE* file = fopen(log_file_location,"r");

    if (file==NULL)
    {
        file = fopen(log_file_location,"w");
        if (file==NULL)
        {
            perror("LOG : log file creation 1: ");
            return -1;
        }
        fclose(file);
        return 1;
    }
    char buffer[MAX_CMD_LEN];
    
        while(fgets(buffer,MAX_CMD_LEN,file)!=NULL)
        {
            
            if (log_size > 15)
            {
                fprintf(stderr,RED"log file error : contains more than 15 commands"RESET);
            }
            // buffer[strcspn(buffer, "\n")] = '\0';
            log_cmds[log_size] = strdup(buffer);
            if (log_cmds[log_size]==NULL)
            {
                perror("LOG: log command allocation");
                return -1;
            }
            // printf("%s",log_cmds[log_size]);
            log_size++;
        }
}




int log_save_to_file()
{
      FILE* file = fopen(log_file_location,"w");
    if (file==NULL)
    {
        perror("LOG: creation or opening log file : 2 :recent command executed but not logged\n");
        return -1;
    }

    for(int i=0;i<log_size;i++)
    {
        if (log_cmds[i]==NULL)
        {
            fprintf(stderr,RED"Something very wrong: log_size variable was mishandled:\n"RESET);
            return -1;
        }
        fprintf(file,"%s",log_cmds[i]);
    }
    fclose(file);
}


int log_save(char* line)
{
    if (line==NULL)
    {
       return 0;
    }
    if (strstr(line,"log")!=NULL)
    {
        return 0;
    }

    // line[strcspn(line, "\n")] = 0;

    if (strcmp(line,"exit\n")==0)
    {
       
        return 0;
    }
    if (log_size>0 && log_cmds[log_size-1]!=NULL && strcmp(log_cmds[log_size-1],line)==0)
    {
        return 0;
    }
   
    
    if (log_size > 15)
    {
        fprintf(stderr,RED"Log size beyond 15 : some error : recent command executed but not logged\n"RESET);
        return -1;
    }
    else if (log_size == 15)
    {

        if (log_cmds[0]!=NULL) 
        free(log_cmds[0]);

       for(int i=0;i<14;i++)
       {
            log_cmds[i] = log_cmds[i+1];
       }
       
       log_cmds[14] = strdup(line);
       if (log_cmds[14]==NULL)
       {
            perror("LOG: allocation error\n");
            return -1;
       }
    }
    else
    {
        log_cmds[log_size] = strdup(line);
        if (log_cmds[log_size]==NULL)
        {
            perror("LOG: allocation error\n");
            return -1;
        }
        log_size++;
    }
    log_save_to_file();
   return 1;
}





// Destroy all evidence of log and clean the arrray
int log_purge()
{
    FILE* file = fopen(log_file_location,"w");
    if (file==NULL)
    {
        perror("LOG: Purge : creation or opening log file : 3\n");
        return -1;
    }
    fclose(file);
    for(int i=0;i<15;i++)
    {
        if (log_cmds[i]!=NULL)
        {
            free(log_cmds[i]);
            log_cmds[i] = NULL;
        }
    }
    log_size = 0;
    return 1;
}



int log_execute(int number)
{
    if (log_size==0)
    {
        fprintf(stderr,RED"No commands in log\n"RESET);
        return 1;
    }
    if (number > log_size || number <=0 )
    {
        fprintf(stderr,RED"Invalid log number\n"RESET);
        return -1;
    }
    char* line = strdup(log_cmds[log_size-number]);
    char* lcopy = strdup(line);
    if (line==NULL)
    {
        perror("LOG: allocation error\n");
        return -1;
    }
    cmd** commands = parse_input(line);
    if (commands==NULL)
    {
        perror("LOG: command parsing error\n");
        return -1;
    }
    if (execute(commands) < -1) {log_save(lcopy);if (lcopy!=NULL) free(lcopy);return -1;}
    log_save(lcopy);
    if (lcopy!=NULL) free(lcopy);
    return 1;

}


// logs the line to the array and updates the file
int log_b(cmd* command)
{
    if (command==NULL || command->tokens==NULL || command->tokens[0]==NULL) return -1;
    char** tokens = command->tokens;
    int i = 1;
    int n_counts = 0;

    while (tokens[n_counts]!=NULL)
    {
        n_counts++;
    }
    if (n_counts>3)
    {
        fprintf(stderr,RED"Log : Too many arguments\n"RESET);
        return -1;
    }
    if (tokens[i]==NULL)
    {
        for (int j=0;j<log_size;j++)
        {
            if (log_cmds[j]==NULL)
            {
                printf("Something very wrong: log_size variable was mishandled:\n");
                return -1;
            }
            printf("%s",log_cmds[j]);
        }
        return 1;
    }
        if (n_counts==2)
        {
              if (strcmp(tokens[1],"purge")==0)
                {
                    return log_purge();
                }
               else{
                    fprintf(stderr,RED"Incorrect arguments after log. See help\n"RESET);
                    return -1;
                }
        }
        else if (n_counts==3)
        {
            if (strcmp(tokens[1],"execute")==0)
                {
                    if (tokens[2]==NULL)
                    {
                        fprintf(stderr,RED"Number expected after execute"RESET);
                        return -1;
                    }
                    int titr = 0;
                    while(tokens[2][titr]!='\0')
                    {
                        if (!isdigit(tokens[2][titr]))
                        {fprintf(stderr,RED"Please don't troll. Give a proper integer after execute\n"RESET);
                        return -1;}

                        titr++;
                    }

                    int num = atoi(tokens[2]);
                    return log_execute(num);
                
                }
        else
        {
            fprintf(stderr,RED"Incorrect arguments after log. See help\n"RESET);
            return -1;
        }
        }      
              
    return 1;
}
      
       

   



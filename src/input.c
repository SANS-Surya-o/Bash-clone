#include "shell.h"
#include "input.h"
#include "builtins.h"
#include "log.h"
#include "directory.h"
#include <libgen.h>    // Used by dirname 



char* read_input()
{
  ssize_t bufsize = 0; // have getline allocate a buffer
  char* line = NULL;
  int result;
  while(( (result = getline(&line, &bufsize, stdin)) == -1) && (errno==EINTR)){
    continue;
   }
   
   if (result == -1) {
    if (feof(stdin)) {

      perror("ctlr-D : Exiting shell");
      exit(EXIT_SUCCESS);    
      

    } else  {

      perror("read_input: readline");
      exit(1);
    }
  // exit(EXIT_SUCCESS);
     
  }
  return line;
}





cmd** parse_input(char* read_line)
{
    if (read_line==NULL)
    {
      return NULL;
    }
    char* line = strdup(read_line);
    if (line==NULL)
    {
      return NULL;
    }
    cmd** commands = (cmd**)malloc(sizeof(cmd*)*MAX_COMMANDS);
    for (int i=0;i<MAX_COMMANDS;i++)
    {
      commands[i] = NULL;
    }
    if (commands==NULL)
    {
      perror("malloc:commands");
      return NULL;
    }
    char *str1=NULL, *str2=NULL, *token=NULL, *subtoken=NULL;
    char *saveptr1=NULL, *saveptr2=NULL;


    int cmd_pos = 0; 
    int white_space_found=0;
    int first_token = 1;
    for (str1 = line; ; str1 = NULL) {
        token = strtok_r(str1, ";" , &saveptr1);
        if (token == NULL)
            {
              break;
            }
        if (first_token && token!=line)
        {
          if (line!=NULL) free(line);
          fprintf(stderr,"Syntax error near unexpected token `;'\n");
          memory_cleanup(commands);
          return NULL;
        }
        first_token = 0;
        if (white_space_found)
            {
              fprintf(stderr,"Syntax error near unexpected ;\n");
              if (line!=NULL){ free(line);line=NULL;}
              return NULL;
            }
          
        // printf("%d: %s\n", 0, token);

        int k = 0;
        white_space_found = 0;
        int first_subtoken = 1;
        for (str2 = token; ; str2 = NULL,k++) {
            subtoken = strtok_r(str2, "&", &saveptr2);
           
            if (subtoken == NULL)
                {break;}
            if (first_subtoken && subtoken!=token)
            {
              if (line!=NULL)
              free(line);
              memory_cleanup(commands);
              fprintf(stderr,"Syntax error near unexpected token `&'\n");
              return NULL;
            }
            first_subtoken = 0;
             if (white_space_found)
            {
              fprintf(stderr,"Syntax error near unexpected &\n");
              if (line!=NULL){ free(line);line=NULL;}
              memory_cleanup(commands);
              return NULL;
            }
            // printf(" --> %s\n", subtoken);
            if (isempty(subtoken))
            {
              white_space_found = 1;
            }
            if (cmd_pos>=MAX_COMMANDS)
            {
                memory_cleanup(commands);
                perror("MAX_COMMANDS reached when parsing input");
                if (line!=NULL){ free(line);line=NULL;}
                return NULL;
            }
            if(cmd_pos>0 && k>0)
            {
               commands[cmd_pos-1]->background = 1;
            }
            if (!isempty(subtoken))
            {
              if (DEBUGGING_MODE)
              {
                printf("parse_input : subtoken : %s\n",subtoken);
              }
              commands[cmd_pos] = create_cmd(subtoken,0);
              if (DEBUGGING_MODE)
              {
                printf("parse_input : commands[%d]->command : %s\n",cmd_pos,commands[cmd_pos]->command);
              }
              if (commands[cmd_pos]==NULL)
              {
                return NULL;
              }
              cmd_pos = cmd_pos + 1;
            }

        }
              
        if (k==0)
        {
          memory_cleanup(commands);
          perror("syntax error near unexpected token `;'");
          if (line!=NULL){ free(line);line=NULL;}
          return NULL;
        }
    }
    commands[cmd_pos] = NULL;
    free(line);
    return commands;
}





cmd** extract_cmds_from_input()
{
    char* line = read_input();
    if (line == NULL)
    {
       return NULL;
    }

    log_save(line);

    cmd** r =  parse_input(line);
    free(line);
    return r;
}
    



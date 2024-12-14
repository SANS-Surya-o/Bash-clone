#include "shell.h"
#include "builtins.h"
#include "directory.h"





// Equivalent of cd command
int hop(cmd* command)
{
    char** tokens = command->tokens;
    char cwd[PATH_MAX];
    getcwd(cwd, sizeof(cwd));
    if (tokens==NULL || tokens[0]==NULL) return -1;
    if (tokens[1]==NULL)
    {
        if(chdir(home_directory)<0)
        {
            perror("chdir:home_directory");
            return -1;
        }
        printf("\n%s\n\n",home_directory);
        return 1;
    }
    int i=1;
    while(tokens[i]!=NULL)
    {
        
        if (strcmp(tokens[i],".")==0)
        {
            printf("\n%s\n\n",cwd);
            i++;
            continue;
        }
        else if (tokens[i][0]=='~')
        {
           char* full_path = malloc((strlen(tokens[i]) + strlen(home_directory) + 2)*sizeof(char));
           full_path = strcpy(full_path,home_directory);
           full_path = strcat(full_path,tokens[i]+1);
           if (chdir(full_path)<0)
           {
                free(full_path);
                printf("hop: %s: No such file or directory\n", tokens[i]);
                return -1;
           }
           free(full_path);
        } 
        else if (strcmp(tokens[i],"-")==0)
        {
            if (prev_directory==NULL)
            {
                printf("hop: cd: OLDPWD not set\n");
                return -1;
            }
            if (chdir(prev_directory)<0)
            {
                perror("chdir:prev_directory");
                return -1;
            }
        }
        else
        {
            if (chdir(tokens[i])<0)
            {
                // perror("chdir");
                printf("hop: %s: No such file or directory\n", tokens[i]);
                return -1;
            }
        }

        if (prev_directory != NULL) {
           free(prev_directory);
           prev_directory = NULL;
        }
        // printf("hop:prev_dir : %s",prev_directory);
        prev_directory = strdup(cwd);
        getcwd(cwd, sizeof(cwd));
        printf("\n%s\n\n",cwd);
        i++;
    }
    return 1;
}


   

    

        

    

        

    

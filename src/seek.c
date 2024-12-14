#define _XOPEN_SOURCE 500
#include "shell.h"
#include "builtins.h"
#include "input.h"
#include "colours.h"
#include "directory.h"
#include <dirent.h>
#include <sys/types.h>
#include <ftw.h>
#include <errno.h>
#include <sys/stat.h>




//GLOBAL VARIABLES
// 
int dflag = 0;
int fflag = 0;
int eflag = 0;
int n_files = 0;
int n_dirs = 0;
char found_path[PATH_MAX];
char* target = NULL;
char* search_dir= NULL;

int seek_flags(char* text,int* dflag, int* fflag, int* eflag)
{
    int i=1;
    while(text[i]!='\0')
    {
        if (DEBUGGING_MODE) printf("%c\n",text[i]);

        if (text[i]=='d')
        {
            *dflag = 1;
        }
        else if (text[i]=='f')
        {
            *fflag = 1;
        }
        else if (text[i]=='e')
        {
            *eflag = 1;
        }
        else
        {
            printf("Invalid flags!\n");
            return false;
        }
        i++;
    }
    return true;
}



              

int search_callback(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    if (search_dir == NULL || target == NULL) return -1;
    const char *filename = fpath + ftwbuf->base;
    const char *relative_path = fpath;
    
    if (strncmp(fpath, search_dir, strlen(search_dir)) == 0) {
        relative_path = fpath + strlen(search_dir);
        
        // Skip the leading '/' if present in relative path
        if (*relative_path == '/' || *relative_path == '\\') {
            relative_path++;
        }
    }

    char formatted_path[1024];
    if (*relative_path != '\0') {
        snprintf(formatted_path, sizeof(formatted_path), "./%s", relative_path);
    } else {
        return 0;
    }
    // 
   if (strncmp(filename,target,strlen(target))==0)
    {
       if ((typeflag == FTW_D) && !fflag) {
            n_dirs++;
            strcpy(found_path, fpath);
            printf("\033[34m%s\033[0m\n", formatted_path); // Print directories in blue
        } else if (typeflag == FTW_F && !dflag) {
            n_files++;
            strcpy(found_path, fpath);
            printf("\033[32m%s\033[0m\n", formatted_path); // Print files in green
        }
    }
    return 0;

}






int seek(cmd* command)
{
    if (command==NULL || command->tokens==NULL || command->tokens[0]==NULL) return -1;
   
    char** tokens = command->tokens;
    int i = 1;
    int target_read = 0;
    int search_dir_read = 0;
    if (target!=NULL) free(target);
    if (search_dir!=NULL) free(search_dir);
    target = NULL;
    search_dir = NULL;
    n_files = 0;
    n_dirs = 0;
    found_path[0] = '\0';
    eflag = 0;
    dflag = 0;  
    fflag = 0;


    while(tokens[i]!=NULL)
    {
        if (tokens[i][0]=='-' && tokens[i][1]!='\0')
        {
            if (!seek_flags(tokens[i],&dflag,&fflag,&eflag))
            {
                return -1;
            }
        }
        else
        {  
            if (!target_read)
            {
                target = strdup(tokens[i]);
                target_read = 1;
            }
            else if (!search_dir_read)
            {
                search_dir = relpath(tokens[i]);
                // printf("search_dir : %s\n",search_dir);
                search_dir_read = 1;
            }
            else
            {
                printf("seek : Too many arguments\n");
                return -1;
            }
           
        }
        i++;
    }

    if (dflag && fflag)
    {
        printf("Invalid flags\n");
        return -1;
    }
    if (target==NULL) 
    {
        printf("Provide valid target\n");
        return -1;
    }
    if (search_dir==NULL)
    {
        search_dir = getcwd(NULL,0);
    }
    if (nftw(search_dir,search_callback,20,0)==-1)
    {
        perror("seek:");
        return -1;
    }

    if (n_files==0 && n_dirs==0)
    {
        printf("No match found!\n");
        return 1;
    }

    if (eflag) {
        if ((n_files== 1 && n_dirs == 0) || (n_files == 0 && n_dirs == 1)) {
            if (n_files == 1) {
                // Print file contents
                FILE *file = fopen(found_path, "r");
                if (file == NULL) {
                    printf("Missing permissions for task!\n");
                    return 1;
                }

                char ch;
                while ((ch = fgetc(file)) != EOF) {
                    putchar(ch);
                }
                fclose(file);
            } else if (n_dirs == 1) {
                // Change directory
                if (prev_directory != NULL) {
                    free(prev_directory);
                    prev_directory = NULL;
                }
                prev_directory = getcwd(NULL, 0);
                if (prev_directory == NULL)
                {
                    perror("strdup: prev_directory");
                    return -1;
                }
                if (chdir(found_path) != 0) {
                    printf("Missing permissions for task!\n");
                    return 1;
                }
               
                printf("Changed directory to: %s\n", found_path);

            }
                
        }
    }
    return 1;

}


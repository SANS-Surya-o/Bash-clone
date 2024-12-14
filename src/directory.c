#include "shell.h"
#include "input.h"
#include "directory.h"

char* home_directory = NULL;
char* prev_directory = NULL;
char* directory = NULL;

int init_directory(){
    directory = (char *)malloc(sizeof(char)*10);
    directory[0]='~';
    directory[1]='\0';
    home_directory = getcwd(NULL,0);
    if (home_directory==NULL)
    {
        perror("getcwd");
        return -1;
    }
    prev_directory = NULL;
    return 1;
}
   

// Updates the directory which is printed on the temrinal
int directory_update()
{
    FILE * file;
        file = fopen(ENV_FILE, "r");
        if (file == NULL) {
            perror("directory_update : Failed to open environment file");
            return -1;
        }
        char dir[PATH_MAX];
        char prev_dir[PATH_MAX];
        if (fgets(dir, PATH_MAX, file) == NULL) {
            perror("directory_update : Failed to read from environment file");
            return -1;
        }
        dir[strlen(dir)-1] = '\0';
        // printf("directory_update : %s\n",dir);
        if (chdir(dir) == -1) {
            perror("directory_update : Failed to change directory");
            return -1;
        }


        if (fgets(prev_dir, PATH_MAX, file) == NULL) {
            prev_directory = NULL;
        }
        else{
            prev_dir[strlen(prev_dir)-1] = '\0';
            prev_directory = prev_dir;
            if (prev_directory == NULL) {
                perror("Directory update failed");
                return -1;
            }
        }
        free(directory);
        directory = get_relative_path(dir);
        if (directory == NULL) {
            perror("Directory update failed");
            return -1;
        }
        fclose(file);
}



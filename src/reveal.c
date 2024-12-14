#include "shell.h"
#include "builtins.h"
#include <dirent.h>
#include <grp.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <pwd.h>
#include "colours.h"
#include "input.h"




bool flags(char* text,int* aflag, int* lflag)
{
    int i=1;
    while(text[i]!='\0')
    {
        if (DEBUGGING_MODE) printf("%c\n",text[i]);

        if (text[i]=='a')
        {
            *aflag = 1;
        }
        else if (text[i]=='l')
        {
            *lflag = 1;
        }
        else
        {
            fprintf(stderr,RED"reveal: invalid option -- '%c'\n"RESET,text[i]);
            return false;
        }
        i++;
    }
    return true;
}

int cmp(const void* p1 ,const void * p2)
{
    return strcmp(*(const char**)p1 , *(const char**)p2);
}

void free_entries(char** entries , int n_memb)
{
    if (entries==NULL) return;
    for(int i=0;i<n_memb;i++)
    {
        if (entries[i]!=NULL)
        {free(entries[i]);entries[i]=NULL;}
    }
    free(entries);
    entries=NULL;
}

void print_file_info(const char *path, int lflag) {
    struct stat fileStat;

    // Get file status information
    if (stat(path, &fileStat) == 0) {
        // Print detailed information if -l flag is set
        if (lflag) {
             printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
            printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
            printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
            printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
            printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
            printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
            printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
            printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
            printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
            printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
            printf(" %ld",fileStat.st_nlink);
            printf(" %s",getpwuid(fileStat.st_uid)->pw_name);

            struct group *grp;

            if ((grp = getgrgid(fileStat.st_gid)) != NULL)
                printf(" %-8.8s", grp->gr_name);
            
            printf(" %ld ",fileStat.st_size);
            
            char buffer[26];  // ctime_r expects a buffer of at least 26 bytes
            char *timeStr = ctime_r(&fileStat.st_mtime, buffer);
           if (timeStr != NULL) {
                timeStr[strlen(timeStr) - 1] = '\0';  // Remove the newline character
                printf(" %s ", timeStr);
            } else {
                printf("Error formatting time\n");
            }

            printf("%s\n", path);  // Print the file name
            printf("\n");
        } else {
            printf("%s\n", path);  // Just print the file name
        }
    } else {
        perror("Error getting file info");
        return;
    }
}

// Print the entries and free memory
int print_entries(char** entries, int aflag, int lflag, int n_memb, char* path)
{
    qsort(entries,n_memb,sizeof(char*),cmp);
    int total = 0;
    for (int i=0;i<n_memb;i++)
    {
        char full_path[PATH_MAX];
        snprintf(full_path,PATH_MAX,"%s/%s",path,entries[i]);
        struct stat fileStat;
        if (stat(full_path,&fileStat) < 0)
        {
            fprintf(stderr,RED"reveal : %s: Could not open\n"RESET,entries[i]);
            continue;
        }
        if (!aflag && entries[i][0]=='.')
        {
            continue;
        }
        total += fileStat.st_blocks;
    }
    printf("total %d\n",total/2);
    for (int i=0; i <n_memb; i++)
    {
        char full_path[PATH_MAX];
        snprintf(full_path,PATH_MAX,"%s/%s",path,entries[i]);
        // printf("%s ",full_path);
        struct stat fileStat;
        if (stat(full_path,&fileStat) < 0)
        {
            printf("reveal : %s: Could not open\n",entries[i]);
            continue;
        }
        if (!aflag && entries[i][0]=='.')
        {
            continue;
        }
        if (lflag)
        {
            printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
            printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
            printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
            printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
            printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
            printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
            printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
            printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
            printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
            printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");
            printf(" %ld",fileStat.st_nlink);
            printf(" %s",getpwuid(fileStat.st_uid)->pw_name);

            struct group *grp;

            if ((grp = getgrgid(fileStat.st_gid)) != NULL)
                printf(" %-8.8s", grp->gr_name);
            
            printf(" %ld ",fileStat.st_size);
            
            char buffer[26];  // ctime_r expects a buffer of at least 26 bytes
            char *timeStr = ctime_r(&fileStat.st_mtime, buffer);
           if (timeStr != NULL) {
                timeStr[strlen(timeStr) - 1] = '\0';  // Remove the newline character
                printf(" %s ", timeStr);
            } else {
                printf("Error formatting time\n");
            }

            if (S_ISREG(fileStat.st_mode))
            {
                if (fileStat.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
                    printf(RED "%s " RESET, entries[i]);}
                else
                    printf(" %s ",entries[i]);
            }
            else if (S_ISDIR(fileStat.st_mode))
            {
               
                 printf(BLUE" %s "RESET,entries[i]);
            }
            printf("\n");
        }
        else
        {
            if (S_ISREG(fileStat.st_mode))
            {
                if (fileStat.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
                    printf(RED "%s " RESET, entries[i]);}
                else
                    printf(" %s ",entries[i]);
            }
            else if (S_ISDIR(fileStat.st_mode))
            {
               
                 printf(BLUE" %s "RESET,entries[i]);
            }
            printf("\n");
        }
         
    }
    printf("\n");
    free_entries(entries, n_memb);
}





int scan_directory(char* path, int aflag, int lflag)
{
    struct stat path_stat;
    char** entries = (char** )malloc(sizeof(char*));
    int num_entries = 0;
    DIR *d;
    struct dirent *dir;

    if (stat(path, &path_stat) != 0) {
        // stat failed, could not access path
        printf("reveal: scan_directory : %s: Could not access : File does not exist or no permission\n", path);
        return -1;
    }

     if (S_ISREG(path_stat.st_mode)) {
        // Path is a file, print its details directly
        print_file_info(path, lflag);
        
    }

    if (S_ISDIR(path_stat.st_mode))
    {
         d = opendir(path);
         if (d) {
        while ((dir = readdir(d)) != NULL) {
            
            entries = realloc(entries, (num_entries+1)*sizeof(char*));
            entries[num_entries] = strdup(dir->d_name);
            num_entries++;
        }

        if (print_entries(entries,aflag,lflag,num_entries,path) < 0){
            closedir(d);
            return -1;
        }
        closedir(d);

        }
        else
        {
            printf("reveal: scan_directory : %s: Could not open directory\n", path);
            return -1;
        }
   
    }
   
    return 1;
}



int reveal(cmd* command)
{
    if (command==NULL || command->tokens==NULL || command->tokens[0]==NULL) return -1;
    char** tokens = command->tokens;
    int i = 1;
    int aflag = 0;
    int lflag = 0;
    int non_flag_arg = 0;
    while(tokens[i]!=NULL)
    {
        if (DEBUGGING_MODE) printf("%s\n",tokens[i]);
        if (tokens[i][0]=='-' && tokens[i][1]!='\0')
        {
            if (!flags(tokens[i], &aflag, &lflag))
            {
                return -1;
            }
            if (DEBUGGING_MODE) printf("aflag:%d -- lflag:%d\n",aflag,lflag);
        }
        i++;
    }

    i=1;
    
    while(tokens[i]!=NULL)
    {
        
        if (tokens[i][0]=='-' && tokens[i][1]!='\0')
        {
            i++;
            continue;
        }
        else
        {  
            // Make this a function later when you have time........................................................................;;;;;;;;;;;;;;;;;;;;;;;;;;;;
            non_flag_arg = 1;
            char* path = relpath(tokens[i]);
            if (path==NULL)
            {
                // printf("reveal: %s: No such file or directory\n",tokens[i]);
                return -1;
            }



            if (scan_directory(path,aflag,lflag)<0)
            {
                free(path);
                path=NULL;
                return -1;
            }
            if (path!=NULL) 
            {
                free(path);
                path = NULL;
            }
        }
       
        i++;
    }

    if (non_flag_arg==0)
    {
        char* path = getcwd(NULL,0);
        if (path==NULL)
        {
            perror("reveal : getcwd : ");
            return -1;
        }
        if (scan_directory(path,aflag,lflag)<0)
        {
            free(path);
            path=NULL;
            perror("getcwd");
            return -1;
        }
        free(path);
        path=NULL;
    }
    return 1;
}
       
       
       
       
       
       
       
       

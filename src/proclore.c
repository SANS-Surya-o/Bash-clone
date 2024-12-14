#include "shell.h"
#include "builtins.h"
#include "directory.h"


/*proclore is used to obtain information regarding a process. If an argument is missing, print the information of your shell.

Information required to print :

pid

Process Status (R/R+/S/S+/Z)

Process group

Virtual Memory

Executable path of process*/


bool isrnum(char* str)
{
    for (int i=0;str[i]!='\0';i++)
    {
        if (!isdigit(str[i]))
        {
            return false;
        }
    }
    return true;
}



int getgpid(int pid)
{
    char path[256];
    char buffer[256];
    char line[256];
    int gid = -1;
    FILE* fp;
    fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Error opening status file");
        return -1;
    }

    // Read the status file line by line
    while (fgets(line, sizeof(line), fp)) {
        // Check if the line starts with "Gid:"
        if (strncmp(line, "Gid:", 4) == 0) {
            // sscanf to extract the real GID from the line
            sscanf(line, "Gid:\t%*d\t%d", &gid);
            break;
        }
    }
    fclose(fp);
    return gid;

}


int get_virtual_memory(int pid)
{
    char path[256];
    char buffer[256];
    char line[256];
    int vmsize = -1;
    FILE* fp;
    sprintf(path, "/proc/%d/status", pid);
    fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Error opening status file");
        return -1;
    }

    // Read the status file line by line
    while (fgets(line, sizeof(line), fp)) {
        // Check if the line starts with "VmSize:"
        if (strncmp(line, "VmSize:", 7) == 0) {
            // sscanf to extract the VmSize from the line
            sscanf(line, "VmSize:\t%d kB", &vmsize);
            break;
        }
    }
    fclose(fp);
    return vmsize;
}

#define EXE_PATH_LEN 1024
char* get_executable_path(int pid)
{
    char path[256];
    char buffer[256];
    char line[256];
    char* exe_path = (char *)malloc(EXE_PATH_LEN*sizeof(char));
    snprintf(path, sizeof(path), "/proc/%d/exe", pid);
    ssize_t len = readlink(path, exe_path, EXE_PATH_LEN - 1);
    if (len != -1) {
        exe_path[len] = '\0';
    }
    else {
        if (exe_path != NULL)
        free(exe_path);
        perror("Error reading link");
        return NULL;
    }
    if (strncmp(exe_path, home_directory, strlen(home_directory)) == 0)
    {
        char* temp = (char *)malloc(sizeof(char) * (strlen(exe_path) - strlen(home_directory) + 20));
        if (temp == NULL)
        {
            perror("malloc: directory updation");
            return NULL;
        }
        temp[0] = '~';
        temp[1] = '\0';
        strcat(temp, exe_path + strlen(home_directory));
        free(exe_path);
        exe_path = temp;
    }
    return exe_path;
}

char get_status(int pid)
{
    char path[40], line[256], status;
    FILE *fp;

    // Construct the path to the /proc/[pid]/stat file
    snprintf(path, sizeof(path), "/proc/%d/stat", pid);

    // Open the stat file for reading
    fp = fopen(path, "r");
    if (fp == NULL) {
        perror("Error opening stat file");
        return '?'; // Return '?' if the file cannot be opened
    }

    // Read the content of the stat file
    if (fgets(line, sizeof(line), fp) != NULL) {
        // sscanf to extract the process status from the third field
        sscanf(line, "%*d %*s %c", &status);
    } else {
        perror("Error reading stat file");
        status = '?';
    }

    fclose(fp);
    return status;
}


bool process_exists(int pid)
{
    char path[256];
    sprintf(path, "/proc/%d", pid);
    if (access(path, F_OK) == 0 && access(path, R_OK) == 0)
    {
        return true;
    }
    return false;
}

int proclore(cmd* command)
{
    if (command==NULL || command->tokens==NULL || command->tokens[0]==NULL) return -1;
    char** tokens = command->tokens;
    if (tokens==NULL)
    {
        return -1;
    }
    int i = 0;
    while(tokens[i]!=NULL)
    {
        i++;
    }
    if (i>2)
    {
        printf("Proclore : Proclore only accepts one argument at max\n");
        return -1;
    }
    if (tokens[1] == NULL)
    {
       
        printf("PID: %d\n", getpid());
        printf("Process Status: %c", get_status(getpid()));
        if (get_status(getpid()) == '?')
        {
            return -1;
        }
        printf("+\n");
        printf("Process Group: %d\n", getpgid(getpid()));
        printf("Virtual Memory: %d\n", get_virtual_memory(getpid()));
        char* exe_path = get_executable_path(getpid());
        if (exe_path == NULL)
        {
            return -1;
        }
        printf("Executable Path: %s\n", exe_path);
        free(exe_path);
    }
    else
    {
        if (!isrnum(tokens[1]))
        {
            printf("Invalid PID\n");
            return -1;
        }
        int pid = atoi(tokens[1]);
         if (pid<=0 || !process_exists(pid))
         {
                printf(RED"Invalid PID\n"RESET);
                return -1;
         }
        printf("PID: %d\n", pid);
        printf("Process Status: %c", get_status(pid));
        if (get_status(getpid()) == '?')
        {
            return -1;
        }
        int pgid = getpgid(pid);    
        int fg_pgid = tcgetpgrp(STDIN_FILENO); 
        if (pgid == fg_pgid)
        {
            printf("+");
        }
        printf("\n");
        printf("Process Group: %d\n", getpgid(pid));
        printf("Virtual Memory: %d\n", get_virtual_memory(pid));
        char* exe_path = get_executable_path(pid);
        if (exe_path == NULL)
        {
            return -1;
        }
        printf("Executable Path: %s\n", exe_path);
        free(exe_path);
    }
    return 1;
}
#include "shell.h"
#include "processManagement.h"
#include "raw_mode.h"
#include "input.h"

#define STAT_FILE_LEN 256

// Function to get the start time from /proc/[pid]/stat
long get_start_time(int pid) {
    char stat_file[STAT_FILE_LEN];
    snprintf(stat_file, sizeof(stat_file), "/proc/%d/stat", pid);

    FILE *fp = fopen(stat_file, "r");
    if (fp == NULL) {
        return -1;  // Could not open the stat file
    }

    long start_time = 0;
    char buffer[1024];

    // Read the stat file content into the buffer
    if (fgets(buffer, sizeof(buffer), fp) != NULL) {
        char *token = strtok(buffer, " ");  // Tokenize the line
        for (int i = 1; i < 22; i++) {      // Skip to the 22nd field
            token = strtok(NULL, " ");
        }
        start_time = atol(token);           // Start time in clock ticks
    }

    fclose(fp);
    return start_time;
}

// Function to find the most recent process by creation time
int get_recent_pid() {
    DIR *proc_dir = opendir("/proc");
    struct dirent *entry;
    int recent_pid = 0;
    long recent_start_time = 0;

    if (proc_dir == NULL) {
        perror("opendir");
        return -1;
    }

    while ((entry = readdir(proc_dir)) != NULL) {
        // Check if the directory name is a number (PID)
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) {
            int pid = atoi(entry->d_name);
            long start_time = get_start_time(pid);

            if (start_time > recent_start_time) {
                recent_start_time = start_time;
                recent_pid = pid;
            }
        }
    }

    closedir(proc_dir);
    return recent_pid;
}


int neonate(cmd* command)
{
    if (command==NULL || command->command==NULL || command->tokens==NULL) {
        return -1;
    }
    int n = 0;
    while (command->tokens[n] != NULL) {
        n++;
    }
    if (n>3)
    {
        printf(RED"neonate: Too many arguments\n"RESET);
        return -1;
    }
    if (n<3)
    {
        printf(RED"neonate: Too few arguments\n"RESET);
        return -1;
    }
    if (strcmp(command->tokens[1], "-n") != 0)
    {
        printf(RED"neonate: Invalid argument: expected -n\n"RESET);
        return -1;
    }
    if (!isnum(command->tokens[2]))
    {
        printf(RED"neonate: Invalid argument: expected a number\n"RESET);
        return -1;
    }
    int time_arg = atoi(command->tokens[2]);
    char ch;
    fd_set read_fds;
    struct timeval tv;

    enable_raw_mode();

    while (1)
    {
        int recent_pid = get_recent_pid();
        if (recent_pid != -1) {
            printf("Most recent process ID: %d\n", recent_pid);
        } else {
            printf("Error retrieving recent PID.\n");
        }

        // Set the sleep duration
        tv.tv_sec = time_arg;
        tv.tv_usec = 0;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);

        int retval = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &tv);

        if (retval == -1) {
            perror("select");
            break;
        } else if (retval) {
            // If input is available, read it
            read(STDIN_FILENO, &ch, 1);
            if (ch == 'x') {
                printf("Exiting monitor.\n");
                break;
            }
        }
    
    }

    disable_raw_mode();
    return 1;
}

    

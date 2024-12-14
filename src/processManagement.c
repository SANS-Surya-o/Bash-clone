#include "shell.h"
#include "processManagement.h"


process* hash_table[HASH_TABLE_SIZE] = {NULL};
int hash_size = 0; 
process* recent_foreground_proc = NULL; 


process* create_process(int pid, const char *command) {
    
    process *new_process = (process *)malloc(sizeof(process));
    if (new_process == NULL) {
        perror("Failed to allocate memory for new process");
        return NULL;
    }

    new_process->pid = pid;

    new_process->command = strdup(command);
    if (new_process->command == NULL) {
        perror("Failed to allocate memory for process command");
        free(new_process); 
        return NULL;
    }

    new_process->next = NULL; 
    new_process->time = 0;
    new_process->state = PROCESS_RUNNING;

    return new_process; 
}

void destroy_process(process *proc) {
    if (proc == NULL) {
        return; 
    }

   
    if (proc->command != NULL) {
        free(proc->command);
    }

    free(proc);
}



unsigned int hash_function(int pid) {
    return pid % HASH_TABLE_SIZE;
}

process* add_process(int pid, const char *command) {
    
    if (kill(-pid, 0) == -1) {
        return NULL;
    }
    
    hash_size++;
    unsigned int index = hash_function(pid); // Calculate the hash index

    process* new_process = create_process(pid, command);
    if (hash_table[index] == NULL) {
        hash_table[index] = new_process;
    }else
    {
        process *current_process = hash_table[index];
        while (current_process->next != NULL) {
            current_process = current_process->next;
        }
        current_process->next = new_process;
    }
    return new_process;
}


process* find_process(int pid) {
    if (hash_size == 0) {
        return NULL; 
    }
    unsigned int index = hash_function(pid); 
    process *current_process = hash_table[index];

    
    while (current_process != NULL) {
        if (current_process->pid == pid) {
            return current_process; 
        }
        current_process = current_process->next;
    }

    return NULL; // Process not found
}


void delete_process(int pid) {
    
    unsigned int index = hash_function(pid); 
    process *current_process = hash_table[index];
    process *prev_process = NULL;

    while (current_process != NULL) {
        if (current_process->pid == pid) {
            if (prev_process == NULL) {
                // Process is at the head of the list
                hash_table[index] = current_process->next;
            } else {
                // Bypass the current process in the list
                prev_process->next = current_process->next;
            }
            free(current_process->command);
            free(current_process);
            hash_size--;
            return;
        }
        prev_process = current_process;
        current_process = current_process->next;
    }
 
}


void delete_all_processes() {
    hash_size = 0;
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        process *current_process = hash_table[i];
        while (current_process != NULL) {
            process *tmp = current_process;
            current_process = current_process->next;
            free(tmp->command);
            free(tmp);
        }
        hash_table[i] = NULL;
    }
}


void print_all_processes() {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        process *current_process = hash_table[i];
        while (current_process != NULL) {
            printf("PID: %d, Command: %s\n", current_process->pid, current_process->command);
            current_process = current_process->next;
        }
    }
}



process** get_all_processes(int* index) {
    (*index) = 0;
    process** all_processes = (process**)malloc(sizeof(process*) * (hash_size+1));
    if (all_processes == NULL) {
        perror("Failed to allocate memory for all processes");
        return NULL;
    }
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        process *current_process = hash_table[i];
        while (current_process != NULL) {
            all_processes[(*index)++] = current_process;
            current_process = current_process->next;
        }
    }
    all_processes[*index] = NULL;
    return all_processes;
}


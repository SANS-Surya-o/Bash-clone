#ifndef _PRCOESS_MANAGEMENT_H
#define _PRCOESS_MANAGEMENT_H


// Define the process struct
# define PROCESS_RUNNING 1
#define PROCESS_STOPPED 2
typedef struct process {
    int pid;                   
    char *command;   
    int time;          
    struct process *next;       
    int state;
} process;

#define HASH_TABLE_SIZE 4096 

process* add_process(int pid, const char *command);
process* find_process(int pid) ;
void delete_process(int pid) ;
void delete_all_processes() ;
process* create_process(int pid, const char *command);
void destroy_process(process *p) ;
void print_all_processes();

// Dear function which uses me, kindly free only an only the output i Give; i.e free(procs) if procs was returned
process** get_all_processes(int* index);


extern process *hash_table[HASH_TABLE_SIZE];
extern process *recent_foreground_proc;


#endif 




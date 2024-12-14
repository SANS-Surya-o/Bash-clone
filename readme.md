# shell




### Note on Heap memory management:
Special attention has been given to prevent memory leaks in this project. All allocated memory blocks are freed appropriately when they are no longer needed. However, some pointers, such as those for username and systemname, are designed to persist throughout the program’s execution. These pointers are not freed explicitly because the operating system automatically handles their deallocation upon program termination.

As a result, while tools like Valgrind may show a few bytes of memory as reachable, none of it is lost, and no memory leaks should occur.
If any unexpected errors (bugs) arise but thankfully don't cause a crash, memory freeing is not guaranteed.




### Assumptions on memory :
MAX_COMMANDS - 100
- (see `spec 10,11,12` for how compound commands are parsed)
- There can be a maximum of 100 pipeline separated by & or ;.
- Each pipeline can contain upto 100 commands 
- In total, it should be possible to parse 10,000 commands in the same prompt (Don't do that though)

PATH_MAX - 4096 

- 4096
- Any file path is assumed to be smaller than 4096 bytes. Larger pathlengths should ideally cause graceful termination (may lead to crashes)


MAX_CMD_LEN - 1024

- Length of a single command is assumed to be 1024 
-----

##### I implemented backgrounding for all commands (including builtins commands) except fg and log.
fg : fg command has to wait for the process brought to the foreground, and if implemented in a child, it cant wait for processes spawned by its father (the shell process) as waitpid can wait only for its children.

log : log execute command runs commands in a child. So running log in a child would mean log execute cmds would run in grandchildren => they can not be waited for.


### Spec 1: 
 
1. 

### Spec- 2:

1. Characters (& or ;) should not occur contiguously. This prints an error. For eg, `cmd1 & ; cmd2` , `cmd1 && cmd2`,  `cmd1 ;; cmd 2` are invalid.

3. A command can not start (but can end) with ; or &.
   Basically, ; and & are spearators. 


### Spec-3:
- If user never hopped into a different directory but issues a "hop -" : an error is issued. 
- Running hop in background does not change the directory of the parent

### Spec-4:
The output files are in the lexicographic order based on ASCII values of the characters involved. 
The reveal - command gives error: OLDPWD not set if user never changed directory.

### Spec-5:
The commands are stored in a file : log-file.
log file is updated every time a new command is given.
Commands with 'log' in it are not included into the log. Note that this includes those which have log as a substring without it actually being a command.   
Ex: echo "logarithm" is not logged.
Log execute <no> commands are stored in the log file unless it matches the previously stored command

### Spec-6:
In some cases this is leading to synchronisation issues.
If the execution time of foreground command exceeds 2s we are required to print it as we wait for the next user input: I am doing this even if the foreground process terminated with an error but consumed some time. THis feature can be changed easily(with an if condition: if the command fails) and is an implementation choice. 



### Spec-7:
Regarding question 1 from specification 7 in the doubts document: the terms "foreground" and "background" for processes might be unclear. For my shell implementation, I could indicate background processes with a '+' sign because the shell has information about processes running in the background. However, for the original shell, the approach I used involves checking if the process's group ID (gpid) matches that of the terminal.

### SPEC-9:
1. If there exists # in a line from myshrc.txt, it is interpreted as a comment  
2. The syntax for aliasing in the file is ```aliasCommand = actualCommand```
3. The log file stores the actual expansion of the alias for ease of implementation
4. Number of commands which can be piped is only limited by the memory of the system.

### SPEC-10,11,12: 
1. Sources: https://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell  

### NOTE ON PARSING:
- A compound command is a sequence of one or more pipelines separated by one of the operators ‘;’, ‘&’, ‘&&’, or ‘||’, and optionally terminated by one of ‘;’, ‘&’, or a newline.

- For eg, ```<pipeline1> & <pipeline2> ; <pipeline3> ``` is executed as follows : complete pipeline1 runs in background, piepline2 and pipeline3 in foreground sequentially. 

2. To enable this, I implemented backgroudning for builtin commands as well. 

2. Each pipeline is executed concurrently. This is in accordance with actual bash implementation. 


2. In accordance with bash:
```
Shells don’t care where in the command line you put the output file redirection. No matter where in the command line you type it, it has the same effect, though most people put it at the end, as in the first example below:

$ echo hi there mom >file                   # echo has three arguments
$ echo hi there >file mom                   # echo has three arguments
$ echo hi >file there mom                   # echo has three arguments
$ echo >file hi there mom                   # echo has three arguments
$ >file echo hi there mom                   # echo has three arguments

All the above commands cause redirection of the three echo commands "hi there mom" to "file" 
```
2. In accordance with how bash handles commands which have pipes and redirection simulataneously:   
 The shell does the recognizing of pipe characters and splitting a command line into piped commands first, before doing file redirection. File redirection happens second (after pipe splitting), and if present, has precedence over pipe redirection. (The file redirection is done after pipe splitting, so it always wins, leaving nothing for the pipe.)

```
$ ls -l      | wc              # correct - output of ls goes into the pipe
2 11 57

$ ls -l >out | wc              # WRONG! - output of ls goes into the file
0 0 0                          # wc reads an empty pipe and outputs zeroes

This is why in the above pipe wc has no characters to count from ls:

First, the shell splits the command line on the pipe, redirecting the output of the command on the left into the input of the command on the right, without knowing anything about what the commands might be.
Next, the shell does the standard output file redirection on the ls command on the left of the pipe and changes the ls standard output away from the pipe into the file out.
Finally, the shell finds and runs both commands simultaneously:
All the standard output from ls on the left goes into the file out; nothing is available to go into the pipe.
The wc command on the right of the pipe counts an empty input from the pipe and outputs zeroes: 0 0 0
Remember: Redirection can only go to one place, and file redirection always wins over pipes, because it is done after pipe splitting:

$ ls /bin >out                 # all output from ls goes into file "out"
$ ls /bin >out | wc            # WRONG! output goes into "out", not into pipe
0 0 0                          # wc counts an empty input from the pipe

```
4. Piped cmds are run in separate children : so sleep 5 | sleep 5 runs only for 5 seconds approximately(all piped commands are run concurrently).

5. In order to avoid zombie processes, if exit is present in any piped command, the shell exits without performing any operation of the pipeline (only exit is performed).
           
Each pipeline is made a process group and if it is to run in the foreground, it is set as the foreground process group of the terminal.
- ctrl-Z,Ctrl-C would stp all the commands in a pipeline.
Calling fg or bg, would affect the whole pipeline of commands.




### SPEC- 13 : 
- Activities :  processes are printed in lexicographic order of 



Chat-gpt links : 
Make file was obtained from a friend--
https://chatgpt.com/share/c07d37b6-8df3-447d-bd70-ed1c2c022e71  

https://chatgpt.com/share/2d14a179-4dc3-479a-85b1-facaf8f9eed4  

For Neonate : https://chatgpt.com/share/66e46bf4-a580-8011-8710-d57b3cc3245e   

For Raw_mode code and doubts regarding signal handling : https://chatgpt.com/share/66e5ac74-a0ac-8011-b0e7-d62f0baccef8

For piping : https://stackoverflow.com/questions/8082932/connecting-n-commands-with-pipes-in-a-shell

For iMan : https://stackoverflow.com/questions/33032893/fetching-a-web-page-using-c-socket-programming

https://unix.stackexchange.com/questions/743002how-does-bash-recognize-background-child-processes-launched-by-a-foreground-proc










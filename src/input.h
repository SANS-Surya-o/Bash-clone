#ifndef INPUT_H
#define INPUT_H



// #define TOK_BUFSIZE 64
// #define TOK_DELIM " \t\r\n\a;&"

cmd** extract_cmds_from_input();
char* read_input();
cmd** parse_input(char* line);
char* relpath(char* token);


void init_aliases();
char* get_relative_path(char* abs_path);
bool isnum(char* token);
bool isempty(char* token);

#endif



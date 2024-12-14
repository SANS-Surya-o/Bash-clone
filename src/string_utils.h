#ifndef _STRING_UTILS_H_
#define _STRING_UTILS_H_


bool isnum(char* str);
bool isempty(char* str);
char* get_relative_path(char* abs_path);
char* relpath(char* token);
void stripTrailingWhitespace(char *str);
char ** strip_whitespaces(char *line, int replace_alias);

#endif


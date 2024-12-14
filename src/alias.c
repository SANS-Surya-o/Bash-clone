#include "shell.h"
#include "input.h"
#include "string_utils.h"

int alias_found = 1;

// If an alias is found, the old line which was malloced in read_input is freed and the replacement is assigned
char** alias_replace(char* line)
{
   char* line_copy = strdup(line);
   FILE* file = fopen(myshrc_file_location, "r");
    if(file == NULL)
    {
        //  perror("alais_replace : Error opening myshrc file");
        alias_found = 0;
        return NULL;
    }
    alias_found = 1;
    const int BUFFER_LEN = 1024;
    char buffer[BUFFER_LEN];
    while (fgets(buffer,BUFFER_LEN,file)!=NULL)
    {
        if (strstr(buffer,"#")!=NULL)
        {
            // Just a comment
            continue;
        }
        char* alias = strtok(buffer,"=");
        char* alias_expanded = strtok(NULL,"=");
        if (alias==NULL || alias_expanded==NULL)
        {
            fprintf(stderr,RED"alias_replace : Error in reading alias from file\n"RESET);
            return NULL;
        }
        stripTrailingWhitespace(alias);
        stripTrailingWhitespace(line_copy);
        if (strcmp(alias,line_copy)==0)
        {
            free(line_copy);
            fclose(file);
            return strip_whitespaces(alias_expanded,DONT_REPLACE_ALIAS);
        }
    }
    fclose(file);
    free(line_copy);
    return NULL;
}




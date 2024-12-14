#include "shell.h"
#include "string_utils.h"
#include "alias.h"



#define TOK_DELIM " \n\t"
#define TOK_BUFSIZE 64



int increase_tokens_size(char*** tokens, int* bufsize)
{
  *bufsize += TOK_BUFSIZE;
  *tokens = realloc(*tokens, (*bufsize) * sizeof(char*));
  if (!(*tokens)) {
    perror("Allocation error in token creation input");
    return -1;
  }
}




void fill_alias_tokens(char** tokens, int * position)
{
  if (tokens==NULL || *position<0)
  {
    return;
  }
  char** alias_replace_tokens = alias_replace(tokens[*position]);
  if (alias_replace_tokens==NULL || alias_replace_tokens[0]==NULL || alias_replace_tokens[0][0]=='\0')
  {
    // printf("NULL");
    return;
  }
  // printf("JALL");
  // printf("alais _ cmds : %s",alias_replace_tokens[0]);
  free(tokens[*position]);
  tokens[*position] = NULL;
  int i = 0;
  while (alias_replace_tokens[i]!=NULL)
  {
    // printf("Alias replace token : %s\n",alias_replace_tokens[i]);
    tokens[*position] = alias_replace_tokens[i];
    (*position)++;
    if (*position>=TOK_BUFSIZE)
    {
      if (increase_tokens_size(&tokens,position)==-1)
      {
        return;
      }
    }
    i++;
  }
  free(alias_replace_tokens);
  (*position)--;
}
 




// DOes not alter the char* line => free the output of this function as follows :
// free(tokens[i]) for all i ; free(tokens); 
char ** strip_whitespaces(char *line, int replace_alias)
{
  int bufsize = TOK_BUFSIZE, position = 0;
  char **tokens = malloc(bufsize * sizeof(char*));
  char *token;
  char* saveptr;
  // printf("strip");
    if (!tokens) {
    free(tokens);
    perror("Allocation error in token creation");
    return NULL;
  
  }
  char* line_copy = strdup(line);

  if (line_copy==NULL)
  {
    free(tokens);
    perror("strip_whitespaces: strdup: line");
    return NULL;
  }
  
  token = strtok_r(line_copy, TOK_DELIM,&saveptr);
  while (token != NULL) {

    tokens[position] = strdup(token);
    if (REPLACE_ALIAS)
    {
      fill_alias_tokens(tokens,&position);
    }
    position++;

    if (position >= bufsize) {
      if (increase_tokens_size(&tokens,&bufsize)==-1)
      {
        return NULL;
      }
    }
    token = strtok_r(NULL, TOK_DELIM, &saveptr);
  }

  tokens[position] = NULL;
  free(line_copy);
  return tokens;
}


void stripTrailingWhitespace(char *str) {
    if (str == NULL || *str == '\0') {
        return;  
    }

    size_t len = strlen(str);

    // Start from the end of the string and move backwards until a non-whitespace character is found
    while (len > 0 && isspace((unsigned char)str[len - 1])) {
        len--;
    }

    // Null-terminate the string at the new length
    str[len] = '\0';
}



char* relpath(char* token)
{
    char* path;
    if (token==NULL)
    {
        return NULL;
    }

    if (strcmp(token,"~")==0)
    {
        path = strdup(home_directory);
    }
    else if (token[0]=='~')
    {
        path = (char*)malloc(strlen(home_directory)+strlen(token)+100);
        path = strcpy(path,home_directory);
        if (path==NULL)
        {
            fprintf(stderr,"relpath : Error when processing path");
            return NULL;
        }
         path = strcat(path,token+1);
    }
    else if (strcmp(token,".")==0)
    {
        path = getcwd(NULL,0);
    }
    else if (strcmp(token,"..")==0)
    {
        char* dircopy = getcwd(NULL,0);
        if (dircopy==NULL)
        {
            perror("getcwd");
            return NULL;
        }
        path = dirname(dircopy);
        if (path==NULL)
        {
            perror("relpath : Error when processing path");
            return NULL;
        }
    }
    else if (strcmp(token,"-")==0)
    {
        if (prev_directory==NULL)
        {
            fprintf(stderr,"reveal : No previous directory yet\n");
            return NULL;
        }
        path = strdup(prev_directory);
    }
    else if (token[0]!='/')
    {
      char* dir = getcwd(NULL,0);
      path = (char*)malloc(strlen(dir)+strlen(token)+100);
      path = strcpy(path,dir);
      path = strcat(path,"/");
      path = strcat(path,token);
      
    }
    else
    {
        path = strdup(token);
    }
    if (path==NULL)
    {
      fprintf(stderr,"relpath: Possible errrors : allocation failure\n");
    }
    return path;
}


char* get_relative_path(char* abs_path) {
    // Allocate space for the relative path
    char* rel_path = (char*)malloc(PATH_MAX);
    if (rel_path == NULL) {
        perror("Failed to allocate memory");
        return NULL;
    }

    // Get the home directory of the current user
    const char* home_dir = home_directory;
    if (home_dir == NULL) {
        fprintf(stderr,"Failed to get home dir");
        return NULL;
    }

    // Ensure the absolute path starts with the home directory
    if (strncmp(abs_path, home_dir, strlen(home_dir)) == 0) {
        // Replace the home directory with ~
        snprintf(rel_path, PATH_MAX, "~%s", abs_path + strlen(home_dir));
    } else {
        // Otherwise, just return the absolute path as-is
        snprintf(rel_path, PATH_MAX, "%s", abs_path);
    }

    return rel_path;
}



bool isnum(char* str)
{
     int  itr = 0;
    while (str[itr]!='\0')
    {
        if (str[itr]<'0' || str[itr]>'9')
        {
            return false;
        }
        itr++;
    }
    return true;

}

bool isempty(char* str)
{
  int i=0;
  int n = strlen(str);
  while(i<n)
  {
    if (!isspace(str[i]) && str[i]!='\n')
    {
      return false;
    }
    i++;
  }
  return true;
}

 

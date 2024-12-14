#ifndef _DIRECTORY_H
#define _DIRECTORY_H

extern char* home_directory;
extern char* prev_directory;
extern char* directory;

int init_directory();
int change_directory(char* path);
int directory_update();

#endif 
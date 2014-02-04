/*
 * CS3600, Spring 2013
 * Project 1 Starter Code
 * (c) 2013 Alan Mislove
 *
 * You should use this (very simple) starter code as a basis for
 * building your shell.  Please see the project handout for more
 * details.
 */

#ifndef _3600sh_h
#define _3600sh_h

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

char* get_input(int*);
int execute_cmd(int, char**);
void do_exit();


char* get_arg(int*, int*);
int is_white_space(char);
void empty_input();
char** get_my_args(int*, int*, int*);
void mem_error();
int is_redirection(char*);

#endif 

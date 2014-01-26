/**
 * CS3600, Spring 2013
 * Project 1 Starter Code
 * (c) 2013 Alan Mislove
 *
 * You should use this (very simple) starter code as a basis for 
 * building your shell.  Please see the project handout for more
 * details.
 */

#include "3600sh.h"

#define USE(x) (x) = (x)

int main(int argc, char*argv[]) {
  // Code which sets stdout to be unbuffered
  // This is necessary for testing; do not change these lines
  USE(argc);
  USE(argv);
  setvbuf(stdout, NULL, _IONBF, 0); 
  
  // Main loop that reads a command and executes it
  while (1) {         
    // You should issue the prompt here
    printf("%s@%s:%s> ", getenv("USER"), getenv("HOST"), getenv("PWD"));
    // You should read in the command and execute it here
    char* input;
    input = get_input();
  }

  return 0;
}

// Function which exits, printing the necessary message
//
void do_exit() {
  printf("So long and thanks for all the fish!\n");
  exit(0);
}

char* get_input() {
  char* input;
  int chars_limit = 50;
  input = (char *) calloc(chars_limit, sizeof(char));
  int chars_read = 0;
  char next;
  next = getchar();
  while (next != '\0' && next != EOF && next != '\n') {
    if (chars_read == chars_limit) {
      chars_limit += 50;
      input = (char *) realloc(input, chars_limit);
    }
    input[chars_read] = next;
    next = getchar();
    chars_read++;
  }

  return input;
}

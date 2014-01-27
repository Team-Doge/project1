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
        // gethostname() needs buffer, deal with later
        printf("%s@%s:%s> ", getenv("USER"), getenv("HOST"), getenv("PWD"));
        // You should read in the command and execute it here
        char* input;
        input = get_input();
        int arg_count = count_args(input);
        char* args[arg_count+1];
        get_args(input, args);
//        for(int i = 0; i < arg_count; i++) {
//            printf("arg_count[%d] = %s\n", i, args[i]);
//        }

        int success = execute_cmd(args);
        if (success != 0) {
            printf("An error occured: Exit code %d\n", success);
        }
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
    if (input == NULL) {
        // Memory error
        exit(1);
    }
    int chars_read = 0;
    char next;
    next = getchar();
    while (1) {
        if (next == '\n' || next == EOF) {
            next = '\0';
        }
        if (chars_read == chars_limit) {
            chars_limit += 50;
            input = (char *) realloc(input, chars_limit);
            if (input == NULL) {
                // Memory error
                exit(1);
            }
        }
        input[chars_read] = next;
        if (next == '\0') {
            break;
        }
        next = getchar();
        chars_read++;
    }

    return input;
}

int execute_cmd(char** argv) {
    pid_t pid  = fork();
    if (pid < 0) {
        // Uh oh, an error
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if (pid == 0) {
        // Child process
        int err = execvp(argv[0], argv);
        printf("Error with code: %d\n", err);
        exit(-1);
    }
    else {
        // Wait for the child
        waitpid(pid, NULL, 0);
    }
    return 0;
}

int count_args(char* cmd) {
    int argc = 0;
    int last_char_was_space = 1;
    int last_char_was_backslash = 0;
    // Find out how many arguments there are in the command
    for (unsigned int i = 0; i < strlen(cmd); i++) {
        char c = cmd[i];
        if (last_char_was_backslash) {
            last_char_was_backslash = 0;
            continue;
        } else if (c == '\\') {
            last_char_was_backslash = 1;
        }

        if (c == ' ' || c == '\t') {
            if (last_char_was_space) {
                continue; 
            } else {
                last_char_was_space = 1;
            }
        } else {
            if (last_char_was_space) {
                argc++;
            }
            last_char_was_space = 0;
        }
    }
    // If last character was backslash, then it's a syntax error
    // because we don't support multiple-line commands
    return argc;
}

void get_args(char* cmd, char** argv) {
    int argc = 0;
    int last_char_was_space = 1;
    int last_char_was_backslash = 0;
    unsigned int len = strlen(cmd);
    for (unsigned int i = 0; i < len; i++) {
        char c = cmd[i];
        if (last_char_was_backslash) {
            last_char_was_backslash = 0;
            continue;
        } else if (c == '\\') {
            last_char_was_backslash = 1;
        }

        if (c == ' ' || c == '\t') {
            if (last_char_was_space) {
                continue; 
            } else {
                last_char_was_space = 1;
                cmd[i] = '\0';
            }
        } else {
            if (last_char_was_space) {
                argv[argc] = cmd + i;
//                printf("Remaining string is: %s\n", cmd + i);
                argc++;
            }
            last_char_was_space = 0;
        }
    }
    argv[argc] = NULL;
}

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

    // There's a script to run if the shell is executed with an argument
    if (argc > 1) {
        freopen(argv[1], "r", stdin);
    }

    int is_eof = 0;

    char hostname[64];
    gethostname(hostname, 64);

    // Main loop that reads a command and executes it
    while (!is_eof) {
        // You should issue the prompt here
        // gethostname() needs buffer, deal with later
        // Only print the prompt if the shell is run without a script
        if (argc == 1) {
            printf("%s@%s:%s> ", getenv("USER"), hostname, getenv("PWD"));
        }
        // You should read in the command and execute it here
        char* input;
        input = get_input(&is_eof);
        int arg_count = count_args(input);
        char* args[arg_count+1];
        int did_get_args = get_args(input, args);
        if (did_get_args == 0) {
            int success = execute_cmd(arg_count, args);
            if (success != 0) {
                printf("An error occured: Exit code %d\n", success);
            }
        }
        free(input);
//        for(int i = 0; i < arg_count; i++) {
//            printf("arg_count[%d] = %s\n", i, args[i]);
//        }

        
    }
    do_exit();

    return 0;
}

// Function which exits, printing the necessary message
//
void do_exit() {
    printf("So long and thanks for all the fish!\n");
    exit(0);
}

char* get_input(int* is_eof) {
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
        if (next == EOF) {
            *is_eof = 1;
        }
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

int execute_cmd(int argc, char** argv) {
    if (argv[0] && strcmp(argv[0], "exit") == 0) {
        do_exit();
    }

    pid_t pid  = fork();

    if (pid < 0) {
        // Uh oh, an error
        fprintf(stderr, "Fork Failed");
        return 1;
    }
    else if (pid == 0) { // Child process

        // Checking for stdin/stdout redirection
        for (int i = 0; i < argc; i++) {
            char* arg = argv[i];
            char* redirect_filename;
            if (arg[0] ==  '>' || arg[0] == '<' || (arg[0] == '2' && arg[1] == '>')) {
                if (strlen(arg) > 1 + (arg[0] == '2')) {
                   redirect_filename = arg + 1 + (arg[0] == '2');
                   if (i == 0) {
                    argv++;
                    i--;
                   } else {
                       argv[i] = NULL;
                   }
                } else if (argv[i+1] != NULL) {
                    redirect_filename = argv[i+1];
                    if (i == 0) {
                        argv += 2;
                        argc -= 2;
                        i--;
                    } else {
                        argv[i] = NULL;
                    }
                } else {
                    printf("Syntax error: expected file name after '%c'.\n", arg[0]);
                    exit(-2);
                }
        //        printf("redirect %s\n", arg);
          //      printf("filename %s\n", redirect_filename);
                if (arg[0] == '>') {
                    freopen(redirect_filename, "w", stdout);
                } else if (arg[0] == '2') {
                    freopen(redirect_filename, "w", stderr);
                } else {
                    freopen(redirect_filename, "r", stdin);
                }
            }
        }

        execvp(argv[0], argv);

        if (errno == ENOENT) {
            printf("Error: Command not found.\n");
        } else if (errno == EACCES) {
            printf("Error: Permission denied.\n");
        } else {
            printf("Error: Exit code %d\n", errno);
        }
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
    unsigned int len = strlen(cmd);
    // Find out how many arguments there are in the command
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

int get_args(char* cmd, char** argv) {
    int argc = 0;
    //int last_char_was_space = 1;
    //int last_char_was_backslash = 0;
    unsigned int len = strlen(cmd);

    char last_char = ' ';
    int current_arg_max_size = 30;
    int current_arg_len = 0;
    char* current_arg = (char *) calloc(current_arg_max_size, sizeof(char));

    unsigned int a;
    for (a = 0; a < len; a++) {
        char c = cmd[a];
        //printf("Finding a space for %c\n", c);
        if (last_char != '\\') {
            switch(c) {
                case '\\': 
                    // Do nothing
                    break;
                case '2':
                    if (cmd[a+1] == '>') {
                        // Redirect stderr
                        argv[argc] = "2>";
                        argc++;
                        a++;
                        last_char = '>';
                        continue;
                    } else {
                        cpy_arg_char(current_arg, &current_arg_len, &current_arg_max_size, c);

                        // We've reached the end of all args, add in what is in current_arg to argv
                        if (a + 1 == len) {
                            cpy_arg(current_arg, &argc, argv);
                            current_arg_len = 0;
                        }
                    }
                    break;
                case ' ':
                case '\t':
                    if (last_char == ' ' || last_char == '\t' || last_char == '>' || last_char == '<') {
                        break;
                    }
                    // We've reached the end of an arg, so we want to copy current_cmd into 
                    // a new string, put that string in argv, and then reset current_cmd to be 
                    // empty again so we can read in the next arg
                    char* new_arg = (char *) calloc(current_arg_len + 1, sizeof(char));
                    strcpy(new_arg, current_arg);
                    free(current_arg);
                    current_arg_len = 0;
                    current_arg = (char *) calloc(30, sizeof(char));
                    argv[argc] = new_arg;
                    argc++;
                    break;
                case '>':
                case '<':
                    // We've reached a redirection change, so the current_cmd can be copied into
                    // a new string, put that string in argv, and then reset current_cmd to be 
                    // empty again so we can read in the next arg after the redirection only if there
                    // is a whitespace before it. We shoud also put in the redirection as its own 
                    // separate arg into argv
                    if (last_char == ' ' || last_char == '\t') {
                        char* new_arg = (char *) calloc(2, sizeof(char));
                        strcpy(new_arg, &c);
                        new_arg[1] = '\0';
                        argv[argc] = new_arg;
                        argc++;
                        break;
                    }
                default:
                    cpy_arg_char(current_arg, &current_arg_len, &current_arg_max_size, c);

                    // We've reached the end of all args, add in what is in current_arg to argv
                    if (a + 1 == len) {
                        cpy_arg(current_arg, &argc, argv);
                        current_arg_len = 0;
                    }
                    break;
            }
        } else {
            switch(c) {
                case 't':
                    cpy_arg_char(current_arg, &current_arg_len, &current_arg_max_size, '\t');
                    break;
                case '&':
                case ' ':
                case '\\':
                    cpy_arg_char(current_arg, &current_arg_len, &current_arg_max_size, c);
                    break;
                default:
                    // UH OH. DAT BAD.
                    printf("Error: Unrecognized escape sequence.");
                    return -1;
            }
            // We've reached the end of all args, add in what is in current_arg to argv
            if (a + 1 == len) {
                cpy_arg(current_arg, &argc, argv);
                current_arg_len = 0;
            }
            last_char = ' ';
            continue;
        }
        last_char = c;
    }
   argv[argc] = NULL;
   return 0;
}

void cpy_arg_char(char* current_arg, int* current_arg_len, int* current_arg_max_size, char c) {
    if (*current_arg_len == (*current_arg_max_size - 1)) {
        *current_arg_max_size += 30;
        current_arg = (char *) realloc(current_arg, *current_arg_max_size);
    }
    current_arg[*current_arg_len] = c;
    *current_arg_len += 1;
}

void cpy_arg(char* current_arg, int* argc, char** argv) {
    int current_arg_len = strlen(current_arg);
    char* new_arg = (char *) calloc(current_arg_len + 1, sizeof(char));
    strcpy(new_arg, current_arg);
    free(current_arg);
    current_arg = (char *) calloc(30, sizeof(char));
    argv[*argc] = new_arg;
    *argc += 1;
}


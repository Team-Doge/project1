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
        char** my_argv;
        int count;
        int should_background = 0;
        my_argv = get_my_args(&count, &should_background, &is_eof);
        if (my_argv == NULL) {
            continue;
        }
       // printf("Got %d args.\n", count);
        if (should_background) {
           // printf("Will background the process.\n");
        }
        for (int i = 0; i < count; i++) {
            //printf("Got arg %d: '%s'\n", i, my_argv[i]);
        }

        execute_cmd(count, my_argv);
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

                    int f = open(redirect_filename, O_WRONLY);

                    //FILE* stdout_file;
                    //stdout_file = freopen(redirect_filename, "w", stdout);
                    //printf("stdout_file: %p\n", (void*) stdout_file);
                    if (f < 0) {
                        printf("Error: Unable to open redirection file.\n");
                        exit(errno);
                    }
                    
                    dup2(f, 1);
                } else if (arg[0] == '2') {
                    FILE* stderr_file;
                    stderr_file = freopen(redirect_filename, "w", stderr);
                    if (stderr_file == NULL) {
                        printf("Error: Unable to open redirection file.\n");
                        exit(errno);
                    }
                } else {
                    FILE* stdin_file;
                    stdin_file = freopen(redirect_filename, "r", stdin);
                    if (stdin_file == NULL) {
                        printf("Error: Unable to open redirection file.\n");
                        exit(errno);
                    }
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

char* get_arg(int* status, int* is_eof) {
    char* buffer = (char*) calloc(10, 1);
    char c;
    char next;
    int bufsz = 0;
    int count = 0;
    int do_loop = 1;
    c = getchar();

    // Read through any whitespace
    while (is_white_space(c) && c != '\n') {
        c = getchar();
    }

    // Check for redirection
    if (c == '>' || c == '<') {
        buffer[0] = c;
        count = 1;
        do_loop = 0;
    } else if (c == '2') {
        next = getchar();
        if (next == '>') {
            do_loop = 0;
        } 
        buffer[0] = c;
        count = 1;
        if (!is_white_space(next)) {
            buffer[1] = next;
            count = 2;
        }
        c = next;
    }

    // Parse the argument
    while (c != EOF && !is_white_space(c) && do_loop) {
        if  (c == '\\') {
            next = getchar();
            if (next == EOF) {
                *is_eof = 1;
            }
            // Handle escape cases
            if (next == 't') {
                c = '\t';
            } else if (next == '\\') {
                c = '\\';
            } else if (next == ' ') {
                c = ' ';
            } else if (next == '&') {
                c = '&';
            } else {
                printf("Error: Unrecognized escape sequence.\n");
                if (next == '\n') {
                    *status = 1;
                } else {
                    empty_input();
                }
                free(buffer);
                return NULL;
            }
        } else if (c == '&') {
            *status = 2;
            c = getchar();

            while (is_white_space(c) && c != '\n' && c != EOF) {
                c = getchar();
            }

            if (c != '\n' && c != EOF) {
                printf("Error: Invalid syntax.\n");
                empty_input();
                free(buffer);
                return NULL;
            }

            if (c == EOF) {
                *is_eof = 1;
            }

            break;
        }

        if (count + 1 > bufsz) {
            buffer = realloc(buffer, bufsz + 10);
            if (buffer == NULL) {
                perror("Error resizing buffer for arg.");
                do_exit();
            }
            bufsz += 10;
        }
        buffer[count] = c;
        count++;
        c = getchar();
    }


    // Check to see if we've read in the full line
    if ((c == '\n' || c == EOF) && (*status != 2)) {
        *status= 1;
    }

    if (c == EOF) {
        *is_eof = 1;
    }

    // Save the argument to memory
    char* ret = (char*) calloc(count + 1, 1);
    strncpy(ret, buffer, count);
    free(buffer);

    return ret;
}



int is_white_space(char c) {
    return (c == ' ') || (c == '\t') || (c == '\n');
}


void empty_input() {
    char c;
    c = getchar();
    while (c != '\n' && c != EOF) {
        c = getchar();
    }
}





char** get_my_args(int* count, int* should_background, int* is_eof) {
    int argc = 0;
    int argv_sz = 10;
    int status = 0;
    char* arg;
    char** argv = (char**) calloc(argv_sz, sizeof(char*));
    if (argv == NULL) {
        mem_error();
    }
    
    while (status == 0) {
        arg = get_arg(&status, is_eof);
        if (arg == NULL) {
            free(argv);
            *count = 0;
            return NULL;
        }

        if ((status == 1 || status == 2) && strlen(arg) == 0) {
            break;
        }

        if (argc + 1 > argv_sz) {
            argv_sz += 10;
            argv = realloc(argv, argv_sz * sizeof(char*));
            if (argv == NULL) {
                mem_error();
            }
        }
        argv[argc] = arg;
        argc++;

        //printf("Added arg '%s' with status %d\n", arg, status);
    }

    char** ret = (char**) calloc(argc + 1, sizeof(char*));
    if (ret == NULL) {
        mem_error();
    }

    for (int i = 0; i < argc; i++) {
        ret[i] = argv[i];
    }

    ret[argc] = NULL;
    *count = argc;

    if (status == 2) {
        *should_background = 1;
    }

    return ret;
}


void mem_error() {
    perror("Out of memory.\n");
    do_exit();
}


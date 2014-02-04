/** * CS3600, Spring 2013
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
        execute_cmd(count, my_argv, &should_background);
    }
    do_exit();

    return 0;
}

// Function which exits, printing the necessary message
void do_exit() {
    printf("So long and thanks for all the fish!\n");
    wait(NULL);
    exit(0);
}

/**
  * Execute a command.
  * @var argc The number of arguments
  * @var argv The array of arguments
  * @var should_background Whether or not to run it as a background process
  */
int execute_cmd(int argc, char** argv, int* should_background) {
    if (argv[0] && strcmp(argv[0], "exit") == 0) {
        do_exit();
    }

    pid_t pid  = fork();

    if (pid < 0) {
        // Uh oh, an error with forking
        fprintf(stderr, "Fork Failed");
        return 1;
    } else if (pid == 0) { // Child process
        char* stdin_fname = NULL;
        char* stdout_fname = NULL;
        char* stderr_fname = NULL;
        int bad_syntax = 0;

        // Checking for stdin/stdout redirection
        for (int i = 0; i < argc; i++) {
            char* arg = argv[i];
            // redirection check
            if (is_redirection(arg)) {
                char* next_arg = argv[i+1];

                if (next_arg == NULL || is_redirection(next_arg) || strcmp(next_arg, "&") == 0) {
                    bad_syntax = 1;
                    break;
                }
                
                if ((i + 2) < argc) {
                    char* next_next_arg = argv[i+2];
                    if (next_next_arg != NULL) {
                        if (!is_redirection(next_next_arg) && strcmp(next_next_arg, "&") != 0) {
                            bad_syntax = 1;
                            break;
                        }
                    }
                }

                if (strcmp(arg, ">") == 0) {
                    stdout_fname = argv[i+1];
                } else if (strcmp(arg, "<") == 0) {
                    stdin_fname = argv[i+1];
                } else if (strcmp(arg, "2>") == 0) {
                    stderr_fname = argv[i+1];
                }

                argv[i] = NULL;
            }
        }

        if (bad_syntax == 1) {
            printf("Error: Invalid syntax.\n");
            exit(3);
        }

        // Redirect stdout
        if (stdout_fname != NULL) {
            if (!is_redirection(stdout_fname)
                    && !(strcmp(stdout_fname, "&") == 0)) {

                FILE* stdout_file;
                stdout_file= freopen(stdout_fname, "w", stdout);
                if (stdout_file == NULL) {
                    printf("Error: Unable to open redirection file.\n");
                    exit(errno);
                }
            } else {
                printf("Error: Invalid syntax.\n");
                exit(3);
            }
        }

        // stdin redirection
        if (stdin_fname != NULL) {
            if (!is_redirection(stdin_fname)
                    && !(strcmp(stdin_fname, "&") == 0)) {

                FILE* stdin_file;
                stdin_file = freopen(stdin_fname, "r", stdin);

                if (stdin_file == NULL) {
                    printf("Error: Unable to open redirection file.\n");
                    exit(errno);
                }
            } else {
                printf("Error: Invalid syntax.\n");
                exit(3);
            }
        }

        // stderr redirection
        if (stderr_fname != NULL) {
            if (!is_redirection(stderr_fname)
                    && !(strcmp(stderr_fname, "&") == 0)) {

                FILE* stderr_file;
                stderr_file = freopen(stderr_fname, "w", stderr);
                if (stderr_file == NULL) {
                    printf("Error: Unable to open redirection file.\n");
                    exit(errno);
                }
            } else {
                printf("Error: Invalid syntax.\n");
                exit(3);
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
    } else {
        if (*should_background == 0) {
            // Wait for the child
            waitpid(pid, NULL, 0);
        }
    }

    return 0;
}

// Is the given arg a redirection character
int is_redirection(char* arg) {
    return strcmp(arg, ">") == 0
            || strcmp(arg, "<") == 0
            || strcmp(arg, "2>") == 0;
}


// Get a single argument, returning a status code and setting is_eof if the EOF char
// was read

// STATUS 1 = End of line, execute command
// STATUS 2 = Background the process
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
                    empty_input(is_eof);
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
                empty_input(is_eof);
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


// Is the given char whitespace
int is_white_space(char c) {
    return (c == ' ') || (c == '\t') || (c == '\n');
}


// Read through stdin until the end of line or EOF
// and set EOF if the EOF was read
void empty_input(int* is_eof) {
    char c;
    c = getchar();
    while (c != '\n' && c != EOF) {
        c = getchar();
    }
    if (c == EOF) {
        *is_eof = 1;
    }
}


// Read in all arguments from stdin
// Increment count with the number of args read
// Set should_background to 1 if the argument array returned should be backgrounded
// Set is_eof to 1 if the end of file was reached
char** get_my_args(int* count, int* should_background, int* is_eof) {
    int argc = 0;
    int argv_sz = 10;
    int status = 0;
    int redirect_stdin = 0;
    int redirect_stdout = 0;
    int redirect_stderr = 0;
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

        if (*arg == '>') {
            redirect_stdout++;
        } else if (*arg == '<') {
            redirect_stdin++;
        } else if (strcmp(arg, "2>") == 0) {
            redirect_stderr++;
        }

        argv[argc] = arg;
        argc++;
    }

    char** ret = (char**) calloc(argc + 1, sizeof(char*));
    if (ret == NULL) {
        mem_error();
    }

    for (int i = 0; i < argc; i++) {
        ret[i] = argv[i];
    }

    if (redirect_stdin > 1 || redirect_stdout > 1 || redirect_stderr > 1) {
        printf("Error: Invalid syntax.\n");
        free(argv);
        *count = 0;
        return NULL;
    }

    ret[argc] = NULL;
    *count = argc;

    if (status == 2) {
        *should_background = 1;
    }

    return ret;
}

// There was a memory error. Exit.
void mem_error() {
    perror("Out of memory.\n");
    do_exit();
}


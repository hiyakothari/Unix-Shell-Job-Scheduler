/* 
 * shell.c - Unix Shell Job Scheduler
 * 
 * A shell implementation with advanced job control features including:
 * - Background and foreground job execution
 * - Signal handling (SIGINT, SIGTSTP, SIGCHLD)
 * - Job queue management
 * - Process control commands (fg, bg, jobs, kill)
 *
 * Compile: gcc -Wall -Wextra -g -o shell shell.c
 * Usage: ./shell
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#define MAX_LINE 1024
#define MAX_JOBS 100
#define MAX_ARGS 64

// Job states
typedef enum {
    RUNNING,
    STOPPED,
    DONE
} job_state_t;

// Job structure
typedef struct {
    int job_id;
    pid_t pid;
    job_state_t state;
    char command[MAX_LINE];
} job_t;

// Job queue
job_t jobs[MAX_JOBS];
int job_count = 0;
int next_job_id = 1;

// Signal handling flags
volatile sig_atomic_t fg_pid = 0;

// Function prototypes
void init_shell();
void parse_command(char *line, char **args, int *background);
int execute_command(char **args, int background);
void add_job(pid_t pid, const char *command, job_state_t state);
void remove_job(pid_t pid);
void update_job_state(pid_t pid, job_state_t state);
job_t* find_job_by_pid(pid_t pid);
job_t* find_job_by_id(int job_id);
void list_jobs();
void wait_for_fg(pid_t pid);
int builtin_command(char **args);
void sigchld_handler(int sig);
void sigint_handler(int sig);
void sigtstp_handler(int sig);

int main() {
    char line[MAX_LINE];
    char *args[MAX_ARGS];
    int background;
    
    init_shell();
    
    printf("=== Unix Shell Job Scheduler ===\n");
    printf("Type 'help' for available commands\n\n");
    
    while (1) {
        printf("shell> ");
        fflush(stdout);
        
        // Read command
        if (fgets(line, MAX_LINE, stdin) == NULL) {
            if (feof(stdin)) {
                printf("\n");
                break;
            }
            continue;
        }
        
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        // Skip empty lines
        if (strlen(line) == 0) continue;
        
        // Parse command
        parse_command(line, args, &background);
        
        // Check for builtin commands
        if (builtin_command(args)) {
            continue;
        }
        
        // Execute command
        execute_command(args, background);
    }
    
    return 0;
}

void init_shell() {
    // Set up signal handlers
    signal(SIGCHLD, sigchld_handler);  // Handle child process termination
    signal(SIGINT, sigint_handler);     // Handle Ctrl+C
    signal(SIGTSTP, sigtstp_handler);   // Handle Ctrl+Z
    
    // Initialize job queue
    memset(jobs, 0, sizeof(jobs));
}

void parse_command(char *line, char **args, int *background) {
    int i = 0;
    *background = 0;
    
    // Tokenize input
    char *token = strtok(line, " \t");
    while (token != NULL && i < MAX_ARGS - 1) {
        // Check for background operator
        if (strcmp(token, "&") == 0) {
            *background = 1;
            break;
        }
        args[i++] = token;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;
}

int execute_command(char **args, int background) {
    pid_t pid;
    
    if (args[0] == NULL) return 1;
    
    pid = fork();
    
    if (pid < 0) {
        perror("fork error");
        return 0;
    }
    
    if (pid == 0) {
        // Child process
        // Restore default signal handlers
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGCHLD, SIG_DFL);
        
        // Create new process group for background jobs
        if (background) {
            setpgid(0, 0);
        }
        
        // Execute command
        if (execvp(args[0], args) < 0) {
            printf("Command not found: %s\n", args[0]);
            exit(1);
        }
    } else {
        // Parent process
        if (background) {
            // Background job
            char cmd[MAX_LINE] = "";
            for (int i = 0; args[i] != NULL; i++) {
                strcat(cmd, args[i]);
                strcat(cmd, " ");
            }
            add_job(pid, cmd, RUNNING);
            printf("[%d] %d %s\n", jobs[job_count-1].job_id, pid, cmd);
        } else {
            // Foreground job
            wait_for_fg(pid);
        }
    }
    
    return 1;
}

void add_job(pid_t pid, const char *command, job_state_t state) {
    if (job_count >= MAX_JOBS) {
        printf("Job queue full\n");
        return;
    }
    
    jobs[job_count].job_id = next_job_id++;
    jobs[job_count].pid = pid;
    jobs[job_count].state = state;
    strncpy(jobs[job_count].command, command, MAX_LINE - 1);
    job_count++;
}

void remove_job(pid_t pid) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid) {
            // Shift remaining jobs
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j + 1];
            }
            job_count--;
            break;
        }
    }
}

void update_job_state(pid_t pid, job_state_t state) {
    job_t *job = find_job_by_pid(pid);
    if (job) {
        job->state = state;
    }
}

job_t* find_job_by_pid(pid_t pid) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid) {
            return &jobs[i];
        }
    }
    return NULL;
}

job_t* find_job_by_id(int job_id) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].job_id == job_id) {
            return &jobs[i];
        }
    }
    return NULL;
}

void list_jobs() {
    if (job_count == 0) {
        printf("No jobs\n");
        return;
    }
    
    printf("\nJob ID  PID     State     Command\n");
    printf("------  ------  --------  -------\n");
    for (int i = 0; i < job_count; i++) {
        const char *state_str;
        switch (jobs[i].state) {
            case RUNNING: state_str = "Running"; break;
            case STOPPED: state_str = "Stopped"; break;
            case DONE: state_str = "Done"; break;
            default: state_str = "Unknown";
        }
        printf("[%d]     %d     %s   %s\n", 
               jobs[i].job_id, jobs[i].pid, state_str, jobs[i].command);
    }
    printf("\n");
}

void wait_for_fg(pid_t pid) {
    int status;
    fg_pid = pid;
    
    if (waitpid(pid, &status, WUNTRACED) < 0) {
        if (errno != ECHILD) {
            perror("waitpid error");
        }
    }
    
    fg_pid = 0;
}

int builtin_command(char **args) {
    if (args[0] == NULL) return 0;
    
    // quit/exit command
    if (strcmp(args[0], "quit") == 0 || strcmp(args[0], "exit") == 0) {
        exit(0);
    }
    
    // jobs command - list all jobs
    if (strcmp(args[0], "jobs") == 0) {
        list_jobs();
        return 1;
    }
    
    // fg command - bring job to foreground
    if (strcmp(args[0], "fg") == 0) {
        if (args[1] == NULL) {
            printf("Usage: fg <job_id>\n");
            return 1;
        }
        int job_id = atoi(args[1]);
        job_t *job = find_job_by_id(job_id);
        if (job == NULL) {
            printf("Job [%d] not found\n", job_id);
            return 1;
        }
        
        // Continue if stopped
        if (job->state == STOPPED) {
            kill(job->pid, SIGCONT);
        }
        
        printf("Bringing job [%d] to foreground: %s\n", job_id, job->command);
        wait_for_fg(job->pid);
        remove_job(job->pid);
        return 1;
    }
    
    // bg command - continue stopped job in background
    if (strcmp(args[0], "bg") == 0) {
        if (args[1] == NULL) {
            printf("Usage: bg <job_id>\n");
            return 1;
        }
        int job_id = atoi(args[1]);
        job_t *job = find_job_by_id(job_id);
        if (job == NULL) {
            printf("Job [%d] not found\n", job_id);
            return 1;
        }
        
        if (job->state == STOPPED) {
            kill(job->pid, SIGCONT);
            job->state = RUNNING;
            printf("Job [%d] continued in background: %s\n", job_id, job->command);
        } else {
            printf("Job [%d] is already running\n", job_id);
        }
        return 1;
    }
    
    // kill command - terminate a job
    if (strcmp(args[0], "kill") == 0) {
        if (args[1] == NULL) {
            printf("Usage: kill <job_id>\n");
            return 1;
        }
        int job_id = atoi(args[1]);
        job_t *job = find_job_by_id(job_id);
        if (job == NULL) {
            printf("Job [%d] not found\n", job_id);
            return 1;
        }
        
        kill(job->pid, SIGKILL);
        printf("Job [%d] terminated\n", job_id);
        return 1;
    }
    
    // help command
    if (strcmp(args[0], "help") == 0) {
        printf("\nAvailable commands:\n");
        printf("  <command> &     - Run command in background\n");
        printf("  jobs            - List all jobs\n");
        printf("  fg <job_id>     - Bring job to foreground\n");
        printf("  bg <job_id>     - Continue stopped job in background\n");
        printf("  kill <job_id>   - Terminate a job\n");
        printf("  quit/exit       - Exit shell\n");
        printf("  Ctrl+C          - Interrupt foreground job\n");
        printf("  Ctrl+Z          - Suspend foreground job\n\n");
        return 1;
    }
    
    return 0;  // Not a builtin command
}

// Signal handlers
void sigchld_handler(int sig) {
    int status;
    pid_t pid;
    
    // Reap all terminated/stopped children
    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        job_t *job = find_job_by_pid(pid);
        
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            // Process terminated
            if (job) {
                printf("\n[%d] Done: %s\n", job->job_id, job->command);
                printf("shell> ");
                fflush(stdout);
                remove_job(pid);
            }
        } else if (WIFSTOPPED(status)) {
            // Process stopped
            if (job) {
                update_job_state(pid, STOPPED);
                printf("\n[%d] Stopped: %s\n", job->job_id, job->command);
                printf("shell> ");
                fflush(stdout);
            } else {
                // Foreground job stopped - add to job list
                char cmd[MAX_LINE];
                snprintf(cmd, MAX_LINE, "(foreground job)");
                add_job(pid, cmd, STOPPED);
                printf("\n[%d] Stopped (use 'fg %d' to resume)\n", 
                       jobs[job_count-1].job_id, jobs[job_count-1].job_id);
                printf("shell> ");
                fflush(stdout);
            }
        }
    }
}

void sigint_handler(int sig) {
    // Only forward to foreground process
    if (fg_pid > 0) {
        kill(fg_pid, SIGINT);
    }
    printf("\n");
}

void sigtstp_handler(int sig) {
    // Only forward to foreground process
    if (fg_pid > 0) {
        kill(fg_pid, SIGTSTP);
    }
}
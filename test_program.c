/*
 * test_program.c - Test program for shell job scheduler
 * 
 * This program is used to test the shell's job control features.
 * It prints a counter every 2 seconds and responds to signals.
 *
 * Compile: gcc -Wall -Wextra -g -o test_program test_program.c
 * Usage: ./test_program
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>

volatile sig_atomic_t running = 1;

void sigint_handler(int sig) {
    printf("\nReceived SIGINT (Ctrl+C), exiting...\n");
    running = 0;
}

void sigtstp_handler(int sig) {
    printf("\nReceived SIGTSTP (Ctrl+Z), pausing...\n");
    signal(SIGTSTP, sigtstp_handler);
}

int main() {
    // Set up signal handlers
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    
    printf("Test program started (PID: %d)\n", getpid());
    printf("I will print a message every 2 seconds\n");
    printf("Press Ctrl+C to stop, Ctrl+Z to suspend\n\n");
    
    int count = 0;
    while (running) {
        printf("Count: %d (PID: %d)\n", ++count, getpid());
        fflush(stdout);
        sleep(2);
    }
    
    printf("Test program exiting\n");
    return 0;
}
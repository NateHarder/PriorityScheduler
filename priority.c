#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>  
/*
Authors: Nathaniel Harder, Brendan Embrey
Assignment Number: 2
Date of Submission: 11/5/20
Name of this file: priority.c
Description of the program: Takes input from a text file and creates child
processes that execute prime.o to count and print prime numbers after a given 
prime. Runs The processes in order of their priority and arrival time and 
suspends the processes that need to be stopped for a context switch to another
process. Also terminates the processes when they finish.
*/

/* Struct that holds information about a process. */
struct process {
    int proc_num; // Process Number
    int arrival;  // Arrival Time
    int burst;    // CPU Burst Value
    int prior;    // Priority 
    int pid;      // Process ID
};

/* Array of processes being used. */
struct process *processes; 
/* Pointer to the currently running process. */
struct process *current;
/* Stack array used to track stopped processes. */
struct process waiting[10];
/* Pointer to the next process to be stopped. */
struct process *proc_to_stop;

int time;               // Counter for time elapsed in seconds.
int process_count = 0;  // Number of processes being run.
int context_switch = 0; // Boolean for determining context switches.
int active[10];         // Array tracking the status of each process.
int running = 0;        // Boolean for tracking if processes have begun to run.
int finished = 0;       // Number of finished processes.
int done = 0;           // Boolean for tracking if program is done.
int top = 0;            // Index variable for top position of waiting stack.


/*
Function Name: push_waiting
Input to the method: process struct to be pushed.
Output(Return value): None. 
Brief description of the task: Pushes a process onto the stack.
*/
void push_waiting(struct process  proc) {
    /* If the stack isn't full, increase the top and add the process to it. */
    if(top != 10){
        top++;
        waiting[top] = proc;
    }
}


/*
Function Name: pop_waiting
Input to the method: None.
Output(Return value): None. 
Brief description of the task: Pops the top of the qaiting stack.
*/
void pop_waiting() {
    /* If there are values in the stack, move the top of the stack down. */
    if(top!= -1){
        top--;
    }
}

/*
Function Name: receive
Input to the method: None.
Output(Return value): None. 
Brief description of the task: Reads information from inpyt file and
creates process structs based on the information.
*/
void receive(FILE *file) {
    /* Declare variables for later. */
    struct process temp_array[10];
    char str[50];
    int token;
    int tokens[4];
    for(int i = 0; i < 10; i++) {
        /* Loop through each line and use fgets to get the line and strtok to
        get the first token. */
        if (fgets(str, 50, file) != NULL) {
            char *token_string = strtok(str, "       ");
            token = atoi(token_string);
            tokens[0] = token;
            int j = 1;
            /* Set values in tokens array to next 3 tokens of each line. */
            while (token_string != NULL && j < 4) {
                token_string = strtok(NULL, "       ");
                token = atoi(token_string);
                tokens[j] = token;
                j++;
            }
            /* Create a process struct from the found tokens, add it to a temp 
            array, and increase count of processes */
            struct process p = { .proc_num = tokens[0], .arrival = tokens[1],
             .burst = tokens[2], .prior = tokens[3], .pid = 0};
            temp_array[i] = p;
            process_count++;
        }
    }
    /* Use malloc to create an array for the processes and copy the temp array 
    into it. */
    processes = malloc(process_count*5*sizeof(int));
    for (int i = 0; i < process_count; i++) {
        processes[i] = temp_array[i];
    }
}

/*
Function Name: check_priority
Input to the method: None.
Output(Return value): None. 
Brief description of the task: Checks the current process priority against the others and switches the current process if a context switch is necessary.
*/
void check_priority() {
    /* For all processes, check if there is burst time left and if a context switch is not needed. Then, if a process in the array matches the current process and the current process has no burst left, set the array process to -1 to show it is done. */
    for (int i = 0; i < process_count; i++) {
        if (processes[i].burst > -1 && context_switch == 0) {
            if (processes[i].proc_num == current->proc_num 
            && current->burst <= 0) {
                processes[i].burst = -1;
            /* Then, if an array process has higher priority than the current process and they aren't the same, push the current process onto the wait stack designate it to be stopped, and set the current process to the array process. */
            } else if (processes[i].prior < current->prior 
            && processes[i].proc_num != current->proc_num) {
                push_waiting(*current);
                proc_to_stop = current;
                current = &processes[i];
                /* Check if another array process has the same arrival time as and a higher priority than the first, and if so switch them, put the first on the wait stack, and raise the context switch flag. */
                for (int j = i+1; j < process_count; j++) {
                    if (processes[i].arrival == processes[j].arrival 
                    && processes[i].prior > processes[j].prior) {
                        current = &processes[j];
                        push_waiting(processes[i]);
                        context_switch = 1;
                    /* If another process has an equal priority to the first but a later arrival time, push the second process onto the stack. */
                    } else if (processes[i].arrival < processes[j].arrival
                    && processes[i].prior == processes[j].prior) {
                        push_waiting(processes[j]);
                    }
                    /* Break the loop to begin a context switch. */
                    break;
                }
            }
        }
    }
}

/*
Function Name: switch_context
Input to the method: None.
Output(Return value): None. 
Brief description of the task: Pauses a process and resumes or starts another.
*/
void switch_context() {
    /* Set a message to resuming and switch it to starting if the process is 
    not started. */
    char *message;
    int resume = 1;
    message = "resuming";
    if (active[current->proc_num] == 0) {
        message = "starting";
        resume = 0;
    }

    /* stop first process and resume second process (or start if not active). */
    printf("Suspending process %d (PID %d) and %s process %d (PID %d)\n\n",
            proc_to_stop->proc_num, proc_to_stop->pid, message, current->proc_num, current->pid);
    kill(proc_to_stop->pid, SIGTSTP);
    active[proc_to_stop->proc_num] = 2;
    /* If the second process is created resume it, otherwise it will be started later. */
    if (resume == 1) {
    kill(current->pid, SIGCONT);
    }
    context_switch = 0; // Set context_switch to 0 to signal end of switch.
}

/*
Function Name: terminate_process
Input to the method: None.
Output(Return value): None. 
Brief description of the task: Uses fork to create a process then 
sets the current pid.
*/
void create_process(int pid) {
    /* Flush stdout and create a new process. */
    fflush(stdout);
    pid = fork();
    /* If in child process: */
    if (pid == 0) {
        fflush(stdout);
        /* Assign process number and priority strings taken from values in 
        current struct */
        char numstr[10];
        int proc_num = current->proc_num;
        sprintf(numstr, "%d", proc_num);
        int prior = current->prior;
        char priorstr[10];
        sprintf(priorstr, "%d", prior);
        /* Execute child process for finding primes. */
        execlp("./prime.o", "./prime.o", numstr, priorstr, NULL);
    } else {
        /* Set current pid to the one returned from fork, and if the program 
        has not started running processes yet, print a message signaling
        the beginning. */
        current->pid = pid;
        if (running == 0) {
            running = 1;
            printf("Scheduler: Time now: %d seconds\n", time);
            printf( "Scheduling to process %d (PID %d)\n\n", current->proc_num, current->pid);
        }
    }
}


/*
Function Name: terminate_process
Input to the method: None.
Output(Return value): None. 
Brief description of the task: Terminates processes and prepares the next 
process in the waiting list.
*/
void terminate_process() {
    /* Print a terminate message, kill the active process, set its activity 
    value to 0 (inactive), and increase the number of finished processes. */
    printf("Scheduler: Time now: %d seconds\n", time);
    printf("Terminating process %d (PID %d)\n\n", current->proc_num, current->pid);
    kill(current->pid, SIGTERM);
    active[current->proc_num] = 0;
    finished++;

    /* If top of waiting stack is over zero (there is a process on the stack),
    pop it and change the current process to it. */
    if (top > -1) {
        if (waiting[top].prior !=0) {
            *current = waiting[top];
            pop_waiting();
        }
    }
    /* If all processes are done, set done to 1 and exit. */
    if (finished >= process_count) {
        done = 1;
    } 
}


/*
Function Name: timer_handler
Input to the method: None.
Output(Return value): None. 
Brief description of the task: Handles new processes by arrival time and priority,
creates new processes, makes contexts witches, resumes paused processes, and 
terminates finished ones.
*/
void timer_handler() {
    if (done == 0) {
        /* If not finished increase time and decrease current burst,
        then check if burst is finished and if so set priority to minimum. */
        time++;
        current->burst--;
        if (current->burst < 0) {
            current->prior = process_count + 1;
        }
        int pid;
        /* If the processes have begun to run, manage current process based on
        priority and arrival time. */
        if (running == 1) {
            check_priority();
        }
        /* If the next process in the waiting list is paused, send a continue signal. */
        if (active[current->proc_num] == 2) {
            kill(current->pid, SIGCONT);
        }
        /* If process is uncreated and has burst left, create and run it. */
        if (current->pid == 0 && current->burst > -1) {
            create_process(pid);
        }
        // If in parent process.
        if(pid != 0) {
            /* If a context switch is needed, perform one. */
            if(context_switch == 1) {
                switch_context();
            }
            /* List the current process as being active and terminate if burst is finished. */
            active[current->proc_num] = 1;
            if (current->burst <= 0) {
                terminate_process();
            }   
        }
    }
}

/*
Function Name: main
Input to the method: argument string for filename.
Output(Return value): returns 0 if program executes successfully. 
Brief description of the task: reads process information from file and sets up 
processes array, sets up timer, and enters a loop to create and handle
processes before exiting.
*/
int main(int argc, char **argv){ 
    /* Open file, read processes from it and creates a process array and 
    assigns current to its first element. */
    char *filename = argv[1];
    FILE *file = fopen(filename, "r");
    receive(file);
    current = &processes[0];
    
    struct itimerval timer;
    struct sigaction sa;
    /* Install timer_handleras the signal handler for SIGALRM. */
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &timer_handler;
    sigaction (SIGALRM, &sa, NULL);
    
    /* The timer goes off 1 second after installation of the timer. */
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec =0;
    /* ... and every 1 secondafter that. */
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec =0;
    /* Start a reaktimer. It counts down whenever this process isexecuting. */
    setitimer (ITIMER_REAL, &timer, NULL);
    
    /* Wait for done boolean to trigger and child processes to finish, then 
    free the malloc'd process array and exit. */
    while (done == 0) {}
    wait(NULL);
    free(processes);
    printf("\n");
    return 0; 
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>  


void receive(FILE *file);
void sort_processes_arrival();
void timer_handler();

struct process {
    int proc_num;
    int arrival;
    int burst;
    int prior;
    int pid;
};

struct process *processes;
struct process *current;
struct process waiting[10];
struct process *proc_to_stop;

int time;
int process_count = 0;
int context_switch = 0;
int active[10];
int running = 0;
int finished = 0;
int done = 0;
int top = 0;

int main(int argc, char **argv){ 
    int child_pid;
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
    while (done == 0) {}
    free(processes);
    exit(0); 
}


void push_waiting(struct process  proc) {
    if(top != 10){
        top++;
        waiting[top] = proc;
    }
}

void pop_waiting() {
    if(top!= -1){
        top--;
    }
}

void receive(FILE *file) {
    struct process temp_array[10];
    char str[50];
    int token;
    int tokens[4];
    for(int i = 0; i < 10; i++) {
        if (fgets(str, 50, file) != NULL) {
            char *token_string = strtok(str, "       ");
            token = atoi(token_string);
            tokens[0] = token;
            int j = 1;
            while (token_string != NULL && j < 4) {
                token_string = strtok(NULL, "       ");
                token = atoi(token_string);
                tokens[j] = token;
                j++;
            }
            struct process p = { .proc_num = tokens[0], .arrival = tokens[1], .burst = tokens[2], .prior = tokens[3], .pid = 0};
            temp_array[i] = p;
            process_count++;
        }
    }
    processes = malloc(process_count*5*sizeof(int));
    for (int i = 0; i < process_count; i++) {
        processes[i] = temp_array[i];
    }
}

void timer_handler() {
    if (done == 0) {
        time++;
        current->burst--;
        if (current->burst < 0) {
            current->prior = process_count;
        }
        int pid;
        if (running == 1) {
            for (int i = 0; i < process_count; i++) {
                if (processes[i].burst > -1 && context_switch == 0) {
                    if (processes[i].proc_num == current->proc_num && current->burst <= 0) {
                        processes[i].burst = -1;
                    } else if (processes[i].prior < current->prior && processes[i].proc_num != current->proc_num) {
                        push_waiting(*current);
                        proc_to_stop = current;
                        current = &processes[i];
                        for (int j = i+1; j < process_count; j++) {
                            if (processes[i].arrival == processes[j].arrival && processes[i].prior > processes[j].prior) {
                                current = &processes[j];
                                context_switch = 1;
                                push_waiting(processes[i]);
                            } else if (processes[i].arrival < processes[j].arrival && processes[i].prior == processes[j].prior) {
                                //current = &processes[i];
                                push_waiting(processes[j]);
                                context_switch = 1;
                            }
                            break;
                        }
                    }
                }
            }
        }
        if (current->pid == 0 && current->burst > -1) {
            fflush(stdout);
            pid = fork();
            if (pid == 0) {
                fflush(stdout);
                char numstr[10];
                int proc_num = current->proc_num;
                sprintf(numstr, "%d", proc_num);
                int prior = current->prior;
                char priorstr[10];
                sprintf(priorstr, "%d", prior);
                execlp("./prime.o", "./prime.o", numstr, priorstr, NULL);
            } else {
                current->pid = pid;
                running = 1;
                printf("Scheduler: Time now: %d seconds\n", time);
                printf( "Scheduling process %d (PID %d)\n\n", current->proc_num, current->pid);
            }
        }
        if(pid != 0) {
            if(context_switch == 1) {
                char *message;
                int resume = 1;
                message = "resuming";
                if (active[current->proc_num] == 0) {
                    message = "starting";
                    resume = 0;
                }
                printf( "Suspending process %d (PID %d) and %s process %d (PID %d)\n\n",
                        proc_to_stop->proc_num, proc_to_stop->pid, message, current->proc_num, current->pid);
                kill(proc_to_stop->pid, SIGTSTP);
                //waitpid(proc_to_stop->pid,NULL,0);
                //waitpid(current->pid,NULL,0);
                if (resume == 1) {
                kill(current->pid, SIGCONT);
                } else {
                    printf("Scheduler: Time now: %d seconds\n", time);
                    printf("Scheduling process %d (PID %d)\n\n", current->proc_num, current->pid);
                }
                context_switch = 0;
            }

            active[current->proc_num] = 1;
            if (current->burst <= 0) {
                kill(current->pid, SIGTERM);
                waitpid(current->pid,NULL,0);
                kill(waiting[top].pid, SIGCONT);
                printf("Scheduler: Time now: %d seconds\n", time);
                printf("Terminating process %d (PID %d)\n\n", current->proc_num, current->pid);
                finished++;
                if (top > -1) {
                    if (waiting[top].prior !=0) {
                        *current = waiting[top];
                        pop_waiting();
                    }
                }
                if (finished >= process_count) {
                    done = 1;
                } else if (current->prior == process_count && current->pid != 0) {
                    printf("Scheduler: Time now: %d seconds\n", time);
                    printf( "Scheduling process %d (PID %d)\n\n", current->proc_num, current->pid);
                }
            }   
        }
    }
}


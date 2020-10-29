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

struct process processes[10];
struct process current;
struct process next;


int time;
int running = 0;

void sort_processes_priority() {
    int priority;
    for(int i = 1; i < 10; i++) {
        priority = processes[i].prior;
        int j = i-1;
        int second_priority = processes[j].prior;
        while (j >= 0 && second_priority > priority) {
            processes[j+1] = processes[j];
            j = j - 1;
        }
        processes[j+1] = processes[i];
    }
}

/*void sort_processes_arrival() {
    int arrival;
    for(int i = 1; i < 10; i++) {
        arrival = processes[i].arrival;
        int j = i-1;
        int second_arrival = processes[j].arrival;
        while (j >= 0  && second_arrival > arrival) {
            if(processes[i].prior == processes[j].prior){
                processes[j+1] = processes[j];
                j = j - 1;
            }
        }
        //processes[j+1] = processes[i];
    }
}*/


int main(int argc, char **argv){ 

    int child_pid;
    char *filename = argv[1];
    FILE *file = fopen(filename, "r");
    receive(file);
    //sort_processes_priority();
    int child_num = 1;
 
    //char* argv[3];
    //argv[0] ="./prime.o";
    //argv[1] ="child_num";
    //argv[2] = NULL;
    struct itimerval timer;
    struct sigaction sa;
    /* Install timer_handleras the signal handler for SIGALRM. */
    memset (&sa, 0, sizeof (sa));
    sa.sa_handler = &timer_handler;
    sigaction (SIGALRM, &sa, NULL);
    
    /* The timer goes off 1 second after installation of the timer. timer.it_value.tv_sec = 1;timer.it_value.tv_usec =0;/* ... and every 1 secondafter that. */
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec =0;
    /* Start a reaktimer. It counts down whenever this process isexecuting. */
    setitimer (ITIMER_REAL, &timer, NULL);
    while (1) {} 
}

void receive(FILE *file) {
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
            processes[i] = p;
        }
    }
}

void timer_handler() {
    if (next == NULL) {
        for (int i = 0; i < 10; i++) {
            if (processes[i].burst != 0) {
                next = processes[i];
                break;
            }
        }
    }
    if(next != current){
        if (current.proc_num !=0) {
            kill(current.proc_num, SIGTERM);
        }
        if(next.pid == 0) {
            int proc_id = fork();
            if (proc_id == 0) {
                execlp("./prime.o", "./prime.o",next.proc_num, next.prior);
            } else {
                waitpid(proc_id,NULL,0);
                next.pid = (unsigned int) proc_id;
            }
        } else {
            kill(next.proc_num, SIGCONT);
        }
        if (!running) {
            running = 1;
            printf("Scheduler: Time now: %d seconds\n", time);
			printf("Scheduling process %d (PID %d)\n\n", next.proc_num, next.pid);
        } else {
            printf("Scheduler: Time now: %d seconds\n", time);
        }
        time++;
        next.burst--;
        current = next;
    } else if (running && current.burst == 0) {
        kill(current.pid, SIGTERM);
		current.pid = 0;
        printf("Scheduler: Time now: %d seconds\n", time);
		printf("Terminating process %d (PID %d)\n\n", current.proc_num, current.pid);
    }
}




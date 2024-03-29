#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<string.h>
#include<limits.h>
/*
Authors: Nathaniel Harder, Brendan Embrey
Assignment Number: 2
Date of Submission: 11/5/20
Name of this file: prime.c

The functions stop_handler, cont_handler, and term_handler receive stop, continue, and terminate signals and print an appropriate message showing process information and the calculated prime. They are grouped together because each one handles one of the aforementioned handles that the program may receive.

The function check_prime_and_print runs a loop to determine if a number is prime. It is grouped with main because main is the only function that calls it.

The function main sets up the above handler functions, then prints a start message and loops to call check_prime_and_print with increasingly high numbers to repeatedly keep finding the next prime number. It is grouped with check_prime_and_print because main is the function that calls check_prime_and_print.

*/

/* Global variables for process number, PID, and priority. */
char *num;
int pid;
char *prior;

/* The initial prime to start counting from. */
long long unsigned int calculated_prime = 2;

/*
Function Name: stopHandler
Input to the method: int from signal.
Brief description of the task: handles SIGTSTP signals
and prints a message showing the calculated prime.
*/      
void stop_handler(int signum) {
    /* Print message showing process numbe, priority, PID, and the highest calculated prime number. */
    printf( "Process %s: My Priority is %s, my PID is %d: I am about to be\n", num, prior, pid);
    printf( "suspended... Highest prime number I found is %llu.\n\n", calculated_prime);
}

/*
Function Name: contHandler
Input to the method: int from signal.
Brief description of the task: handles SIGCONT signals
and prints a message showing the calculated prime.
*/      
void cont_handler(int signum){
    /* Print message showing process numbe, priority, PID, and the highest calculated prime number. */
    printf( "Process %s: My Priority is %s, my PID is %d: I just got resumed.\n",  num, prior, pid);
    printf( "Highest prime number I found is %llu.\n\n", calculated_prime); 
}

/*
Function Name: termHandler
Input to the method: int from signal.
Brief description of the task: handles SIGTERM signals
and prints a message showing the calculated prime.
*/      
void term_handler(int signum) {
    /* Print message showing process numbe, priority, PID, and the highest calculated prime number. */
    printf( "Process %s: My Priority is %s, my PID is %d: I completed my task\n", num, prior, pid);
    printf( "and I am exiting. Highest prime number I found is %llu.\n\n", calculated_prime);
}

/*
Function Name: checkPrimeAndPrint
Input to the method: unsigned long int number to check.
Output(Return value): returns toCheck number if prime or 1 if not.
Brief description of the task: checks if number is prime
and if so return it. Otherwise return 1.
*/      
int check_prime_and_print(long long unsigned int to_check) {
    unsigned long int i = 2; 
    int prime = 1;
    /* Assume number is prime and if it can be divided evenly with any possible
     factor i, set prime to 0 (false) */ 
    while (prime == 1 && i < to_check/2){ 
        if (to_check % i == 0) {
            prime = 0;
        }
        i++;
    } 
    return (prime);
}

/*
Function Name: main
Input to the method: argument string.
Output(Return value): returns 0 if program executes successfully. 
Brief description of the task: handles signals, checks for primes
and prints a starting message.
*/
int main(int argc, char** argv){

    /* Find pid and assign num and prior variables. */
    pid = getpid();
    num = argv[1];
    prior = argv[2];

    /* Set handler for the SIGTSTP signal. */
    struct sigaction tstp;
    memset(&tstp, 0, sizeof(tstp));
    tstp.sa_handler = &stop_handler;
    sigaction(SIGTSTP, &tstp, NULL);

    /* Set handler for the SIGCONT signal. */
    struct sigaction cont;
    memset(&cont, 0, sizeof(cont));
    cont.sa_handler = &cont_handler;
    sigaction(SIGCONT, &cont, NULL);

    /* Set handler for the SIGTERM signal. */
    struct sigaction term;
    memset(&term, 0, sizeof(term));
    term.sa_handler = &term_handler;
    sigaction(SIGTERM, &term, NULL);

    /* Print a message when process starts. */
    printf("Process %s: My Priority is %s, my PID is %d: I just got started.\n", num, prior, pid);
    printf("I am starting with the number %llu to find the next prime number.\n\n", calculated_prime);
    
    int num_printed = 0;
    long unsigned int number_to_check = calculated_prime + 1; 
    /* Loop up to INT_MAX times using the check_prime_and_print function to find the 
    next prime after the previously found prime. */
    while (num_printed < INT_MAX) {
        if (check_prime_and_print(number_to_check) == 1) { 
            calculated_prime = number_to_check;
            num_printed++;
        }
        number_to_check++; 
    } 
}



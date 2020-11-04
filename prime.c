#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<string.h>
/*
Authors: Nathaniel Harder, Brendan Embrey
Assignment Number: 2
Date of Submission: 11/5/20
Name of this file: prime.c
*/
char *num;
int pid;
char *prior;
long long unsigned int nextPrime;

void stopHandler(int signum);
void contHandler(int signum);
void termHandler(int signum);

long long unsigned int calculatedPrime = 123400003;

int checkPrimeAndPrint(unsigned long int toCheck);

/*
Function Name: main
Input to the method: argument string.
Output(Return value): returns 0 if program executes successfully. 
Brief description of the task: handles signals, checks for primes
and prints a starting message.
*/
int main(int argc, char** argv){

    pid = getpid();
    num = argv[1];
    prior = argv[2];

    // Set the action for the SIGTSTP signal
    struct sigaction sigtstp_action;
    memset(&sigtstp_action, 0, sizeof(sigtstp_action));
    sigtstp_action.sa_handler = &stopHandler;
    sigaction(SIGTSTP, &sigtstp_action, NULL);

    // Set the action for the SIGCONT signal
    struct sigaction sigcont_action;
    memset(&sigcont_action, 0, sizeof(sigcont_action));
    sigcont_action.sa_handler = &contHandler;
    sigaction(SIGCONT, &sigcont_action, NULL);

    // Set the action for the SIGTERM signal
    struct sigaction sigterm_action;
    memset(&sigterm_action, 0, sizeof(sigterm_action));
    sigterm_action.sa_handler = &termHandler;
    sigaction(SIGTERM, &sigterm_action, NULL);
    //fflush(stdout);
    printf("Process %s: My Priority is %s, my PID is %d: I just got started.\n", num, prior, pid);
    //fflush(stdout);
    printf("I am starting with the number %llu to find the next prime number.\n\n", calculatedPrime);
    int numPrinted = 0;
    long unsigned int numberToCheck = calculatedPrime +1; 
    while (numPrinted <10) {
        if (checkPrimeAndPrint(numberToCheck) ==1){ 
            nextPrime = numberToCheck;
            numPrinted++;
        }
        numberToCheck++; 
    } 
}

/*
Function Name: stopHandler
Input to the method: int from signal.
Brief description of the task: handles SIGTSTP signals
and prints a message.
*/      
void stopHandler(int signum){
    printf( "Process %s: My Priority is %s, my PID is %d: I am about to be\n", num, prior, pid);
    printf( "suspended... Highest prime number I found is %llu.\n\n", nextPrime);
    //fflush(stdout);
}

/*
Function Name: contHandler
Input to the method: int from signal.
Brief description of the task: handles SIGCONT signals
and prints a message.
*/      
void contHandler(int signum){
    printf( "Process %s: My Priority is %s, my PID is %d: I just got resumed.\n",  num, prior, pid);
    printf( "Highest prime number I found is %llu.\n\n", nextPrime); 
    //fflush(stdout);
}

/*
Function Name: termHandler
Input to the method: int from signal.
Brief description of the task: handles SIGTERM signals
and prints a message.
*/      
void termHandler(int signum) {
    printf( "Process %s: My Priority is %s, my PID is %d: I completed my task\n", num, prior, pid);
    printf( "and I am exiting. Highest prime number I found is %llu.\n\n", nextPrime);
    //fflush(stdout);
}

/*
Function Name: checkPrimeAndPrint
Input to the method: unsigned long int number to check.
Output(Return value): returns toCheck number if prime or 1 if not.
Brief description of the task: checks if number is prime
and if so return it. Otherwise return 1.
*/      
int checkPrimeAndPrint(unsigned long int toCheck){
      unsigned long int i = 2; 
      int prime = 1; 
      while (prime == 1&& i < toCheck/2){ 
              if (toCheck % i == 0) 
                prime = 0;
              i++;
      } 
      return (prime);
}

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
/*
Authors: Nathaniel Harder, Brendan Embrey
Assignment Number: 2
Date of Submission: 11/5/20
Name of this file: prime.c
*/
char *num;
char *pid;
char *prior;
long unsigned int nextPrime;

long unsigned int calculatedPrime = 123400003;
void stopHandler(int signum);
void contHandler(int signum);
void termHandler(int signum);
int checkPrimeAndPrint(unsigned long int toCheck);

/*
Function Name: main
Input to the method: argument string.
Output(Return value): returns 0 if program executes successfully. 
Brief description of the task: handles signals, checks for primes
and prints a starting message.
*/
int main(int argc, char** argv){
int p = getpid();
num = argv[1];
pid = argv[2];
prior = argv[3];
printf("Process %s: My Priority is %s, my PID is %s: I just got started.\n", num, pid, prior);
printf("I am starting with the number %llu to find the next prime number.", calculatedPrime);
signal(SIGTSTP, stopHandler);
signal(SIGCONT, contHandler);
signal(SIGTERM, termHandler);
       int numPrinted = 0;
       long unsigned int numberToCheck = calculatedPrime +1; 
       while (numPrinted <10) {
           if (checkPrimeAndPrint(numberToCheck) ==1){ 
              printf("prime number is %lu \n", numberToCheck);
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
    printf("Process %s: My Priority is %s, my PID is %s: I am about to be\n", num, pid, prior);
    printf("suspended... Highest prime number I found is %llu.", nextPrime);
}

/*
Function Name: contHandler
Input to the method: int from signal.
Brief description of the task: handles SIGCONT signals
and prints a message.
*/      
void contHandler(int signum){
    printf("Process %s: My Priority is %s, my PID is %s: I just got resumed.\n",  num, pid, prior);
    printf("Highest prime number I found is %llu.\n", nextPrime); 
}

/*
Function Name: termHandler
Input to the method: int from signal.
Brief description of the task: handles SIGTERM signals
and prints a message.
*/      
void termHandler(int signum) {
    printf("Process %s: My Priority is %s, my PID is %s: I completed my task\n", num, pid, prior);
    printf("and I am exiting. Highest prime number I found is %llu.\n", nextPrime);
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

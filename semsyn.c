#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>


int counter = 0;
static int volatile count2 = 0;
int  done = 0;
static int volatile total = 0;
//int arr[2];
sem_t full;
sem_t hold;     // used to block all threads
sem_t holdMain; // used to block main thread
sem_t release; //use to block critical section

sem_t countSem;

int checkIfPrimeNum(int numberToCheck)
{
    if (numberToCheck == 2)
        return 1;
    if (numberToCheck == 1)
        return -1;
    for (int i = numberToCheck - 1; i > 1; i--)
    {
        if (numberToCheck % i == 0)
            return -1;
    }
    return 1;
}

void *howManyPrimeNumbers(void *args)
{
int* arr= (int*)args;
int numOfPrime = 0, min, max, sign = 1;
    min = arr[0];
    max = arr[1];

    sem_wait(&hold);
    sem_wait(&full);
    

    for (int i = min; i <= max; i++)
    {
        if (checkIfPrimeNum(i) == 1)
            numOfPrime++;
    }
    sem_wait(&release);
    total += numOfPrime;
    sem_post(&release);
    sem_post(&full);
    sem_wait(&countSem);
    count2++;
    if(count2==counter)
    {
        sem_post(&holdMain);
    }
    sem_post(&countSem);
}

void signal_handler(int signal)
{
    // if SIGINT is received - mark done=1
    // main() will handle the rest
    if (signal == SIGINT)
    {
        done = 1;
        for (size_t i = 0; i < counter; i++)
        {
            sem_post(&hold);
        }
    }
}

void set_sigint(void (*handler)(int))
{
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = handler;
    act.sa_flags = 0;
    if (sigaction(SIGINT, &act, NULL) < 0)
    {
        perror("sigaction() for SIGINT failed");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    if(argc!=2)
    {
        perror("Please enter max number for threads");
        exit(EXIT_FAILURE);
    }
    int n = atoi(argv[1]);
    sem_init(&full, 0, n);
    sem_init(&hold, 0, 0);
    sem_init(&holdMain,0,0);
    sem_init(&release, 0,1);
    sem_init(&countSem,0,1);
    int min, max, arr[2];
    // call signal_handler on every SIGINT
    set_sigint(signal_handler);
    // while SIGINT wasn't received (!done) and there is more input (scanf == 2)
    while ((!done) && (scanf("%d %d", &min, &max) == 2))
    {
    
        arr[0] = min;
        arr[1] = max;
        pthread_t thread;
        pthread_create(&thread, NULL, howManyPrimeNumbers, arr);
        counter++;
    }
    sem_wait(&holdMain);
    printf("\nmain: end. Total: %d\n", total);
    sem_destroy(&holdMain);
    sem_destroy(&full);
    sem_destroy(&hold);
    sem_destroy(&release);
    sem_destroy(&countSem);
    pthread_exit(NULL);
    return 0;
}

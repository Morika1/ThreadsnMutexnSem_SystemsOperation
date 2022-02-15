#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

#define MAX_SIZE 1024

int done = 0;
int counter = 0;
static int volatile isComplete = 0;
static int volatile total = 0;
static int volatile funcCount = 0;
//int arr[2];
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;

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
	int* arr =(int*)args;

    int numOfPrime = 0, min, max;
	min = arr[0];
	max = arr[1];
	pthread_mutex_lock(&lock);
	if (done == 0)
		pthread_cond_wait(&cond, &lock);
	pthread_mutex_unlock(&lock);
	//printf("lock\n");
	
	for (int i = min; i <= max; i++)
	{
		if (checkIfPrimeNum(i) == 1)
			numOfPrime++;
	}

	pthread_mutex_lock(&lock2);
	total += numOfPrime;
	isComplete++;
	if (isComplete == counter)
		pthread_cond_signal(&cond2);
	pthread_mutex_unlock(&lock2);

	return NULL;
}

void signal_handler(int signal)
{
	// if SIGINT is received - mark done=1
	// main() will handle the rest
	if (signal == SIGINT)
	{

		pthread_cond_broadcast(&cond);
		done = 1;
	}
}

void set_sigint(void(*handler)(int))
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
	int min[MAX_SIZE], max[MAX_SIZE];

int arr[2];
	// call signal_handler on every SIGINT
	set_sigint(signal_handler);
	// while SIGINT wasn't received (!done) and there is more input (scanf == 2)
	while ((!done) && (scanf("%d %d", &min[counter], &max[counter]) == 2))
	{
		arr[0] = min[counter];
		arr[1] = max[counter];
		pthread_t p;
		pthread_create(&p, NULL, howManyPrimeNumbers, arr);

		counter++;
	}

	pthread_mutex_lock(&lock2);
	if (isComplete != counter)
		pthread_cond_wait(&cond2, &lock2);
	pthread_mutex_unlock(&lock2);



	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cond);
	pthread_mutex_destroy(&lock2);
	pthread_cond_destroy(&cond2);

	printf("\nmain: end. Total: %d\n", total);

	pthread_exit(NULL);
	return 0;
}
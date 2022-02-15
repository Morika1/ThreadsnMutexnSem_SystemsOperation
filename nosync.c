#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

int done = 0;
static int volatile count = 0;
static int volatile total = 0;
int arr[2];


typedef struct Node
{
    pthread_t thread;
    int min;
    int max;
    int res;
    struct Node *next;
} Node;

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
    int counter = 0, min, max, sign = 1;

    Node *temp = (Node *)args;
    min = temp->min;
    max = temp->max;
    while (!done)
    {
    }
    for (int i = min; i <= max; i++)
    {
        if (checkIfPrimeNum(i) == 1)
            counter++;
    }
    temp->res += counter;
    // printf("count: %d\n", temp->res);
}

void signal_handler(int signal)
{
    // if SIGINT is received - mark done=1
    // main() will handle the rest
    if (signal == SIGINT)
    {
        done = 1;
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
    int min, max;
    Node *head = (Node *)malloc(sizeof(Node));
    Node *tempHead = head;

    // call signal_handler on every SIGINT
    set_sigint(signal_handler);

    // while SIGINT wasn't received (!done) and there is more input (scanf == 2)
    while ((!done) && (scanf("%d %d", &min, &max) == 2))
    {
        Node *temp = (Node *)malloc(sizeof(Node));
        temp->max = max;
        temp->min = min;
        temp->res = 0;
        tempHead->next = temp;
        tempHead = tempHead->next;
        pthread_create(&temp->thread, NULL, howManyPrimeNumbers, temp);
        count++;
    }
    while (head)
    {
        pthread_join(head->thread,NULL);
        total += head->res;
        head = head->next;
    }

    printf("\nmain: end. Total: %d\n", total);
    return 0;
}

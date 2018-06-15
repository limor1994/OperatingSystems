/**
 * Name : Limor Levi
 * Id : 308142389
 */

#ifndef __THREAD_POOL__
#define __THREAD_POOL__

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include "osqueue.h"
//boolean type
#define TRUE 1
#define FALSE 0

#define FAIL -1
#define STDERR_CHANNEL 2


//thread-pool struct
typedef struct thread_pool
{
    //threads
    pthread_t* threads;
    int numThreadsInPool;
    //queue of threads
    struct os_queue* queueOfTasks;
    pthread_mutex_t queueLockMutex;
    //mutex and conditional variable
    pthread_mutex_t lockMutex;
    pthread_cond_t condition;
    //boolean variables
    int isStop;
    int availableToInsertion;
}ThreadPool;

//task struct
typedef struct task
{
    //pointer to function
    void (*function)(void *param);
    //the function's parameters
    void* parameters;
}Task;

//mutex variable to handle multi-threaded programming
pthread_mutex_t mutex;

//functions declaration

/**
 * creates a thread pool struct.
 * @param numOfThreads number of threads in the thread pool.
 * @return reference to new thread pool struct if succeeded, NULL if failed.
 */
ThreadPool* tpCreate(int numOfThreads);

/**
 * Destroys the thread pool.
 * @param threadPool thread pool
 * @param shouldWaitForTasks 0 - dont wait for tasks in the queue, else - wait for tasks.
 */
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

/**
 * inserts a task to the tasks queue of the thread pool.
 * @param threadPool thread pool
 * @param computeFunc task
 * @param param argument to the task
 * @return 0- success , -1 - fail
 */
int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

/**
 * takes a task from the task-queue and execute it with the assistance of threads from the thread pool.
 * @param tp - the thread pool
 */
void* execute(void* tp);

/**
 * the function prints error message in the error channel
 */
void failure();


#endif
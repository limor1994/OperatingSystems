/**
 * Name : Limor Levi
 * Id : 308142389
 */

#include "threadPool.h"

//functions implementation

/**
 * the function creates thread pool
 * @param numOfThreads - the number of threads in the thread pool
 * @return the new thread pool
 */
ThreadPool* tpCreate(int numOfThreads) {
    int i;
    //thread-pool creation
    ThreadPool* threadPool = (ThreadPool*)malloc(sizeof(ThreadPool));
    if (threadPool == NULL) {
        failure();
        return NULL;
    }
    threadPool->numThreadsInPool = numOfThreads;
    //threads creation (for the pool of threads)
    threadPool->threads = (pthread_t*)malloc(sizeof(pthread_t)*threadPool->numThreadsInPool);
    if (threadPool->threads == NULL) {
        failure();
        return NULL;
    }
    //tasks-queue creation
    threadPool->queueOfTasks = osCreateQueue();
    //other thread-pool's variables initialization
    pthread_mutex_init(&(threadPool->lockMutex), NULL);
    threadPool->availableToInsertion = TRUE;
    threadPool->isStop = FALSE;
    //mutexes and conditional-variables initialization
    if ((pthread_mutex_init(&(threadPool->queueLockMutex), NULL) != 0) ||
        (pthread_cond_init(&(threadPool->condition), NULL) != 0) ||
        (pthread_mutex_init(&(threadPool->queueLockMutex), NULL) != 0)) {
        tpDestroy(threadPool, 0);
        failure();
        return NULL;
    }
    //initialization of each thread in separate
    for (i = 0; i < threadPool->numThreadsInPool; i++) {
        if(pthread_create(&(threadPool->threads[i]), NULL, execute, (void *)threadPool) != 0) {
            failure();
        }
    }
    return threadPool;
}


/**
 * inserts a task to the tasks queue of the thread pool.
 * @param threadPool thread pool
 * @param computeFunc task
 * @param param argument to the task
 * @return 0- success , (-1) - fail
 */
int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param) {
    if(threadPool == NULL || computeFunc == NULL || !(threadPool->availableToInsertion)){
        return FAIL;
    }
    //create a new task and insert it to the task-queue
    Task* currentTask = (Task*)malloc(sizeof(Task));
    if (currentTask == NULL) {
        failure();
        return FAIL;
    }
    currentTask->parameters = param;
    currentTask->function = computeFunc;
    osEnqueue(threadPool->queueOfTasks, (void *)currentTask);
    //after insertion of task to the queue - wake up a thread to execute the task
    pthread_mutex_lock(&(threadPool->queueLockMutex));
    if(pthread_cond_signal(&(threadPool->condition)) != 0) {
        failure();
    }
    pthread_mutex_unlock(&(threadPool->queueLockMutex));
    return 0;
}


/**
 * Destroys the thread pool.
 * @param threadPool thread pool
 * @param shouldWaitForTasks 0 - don't wait for tasks in the queue, else - wait for tasks.
 */
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
    if (threadPool != NULL) {
        pthread_mutex_lock(&mutex);
        if (threadPool->availableToInsertion) {
            int i;
            Task *currentTask;
            //mark the thread-pool as not available to insertions
            threadPool->availableToInsertion = FALSE;
            pthread_mutex_unlock(&mutex);

            if (shouldWaitForTasks == FALSE){
                threadPool->isStop = TRUE;
            }
            //notify all the threads to execute their tasks
            pthread_mutex_lock(&(threadPool->queueLockMutex));
            if ((pthread_mutex_unlock(&(threadPool->queueLockMutex)) != 0) ||
                (pthread_cond_broadcast(&(threadPool->condition)) != 0)) {
                failure();
                exit(1);
            }
            //wait for all the threads to finish their tasks
            for (i = 0; i < threadPool->numThreadsInPool; i++) {
                pthread_join(threadPool->threads[i], NULL);
            }
            threadPool->isStop = TRUE;
            //free all the memory that was allocated for the thread pool
            while (!osIsQueueEmpty(threadPool->queueOfTasks)) {
                currentTask = osDequeue(threadPool->queueOfTasks);
                free(currentTask);
            }
            osDestroyQueue(threadPool->queueOfTasks);
            free(threadPool->threads);
            pthread_mutex_destroy(&mutex);
            pthread_mutex_destroy(&(threadPool->lockMutex));
            pthread_mutex_destroy(&(threadPool->queueLockMutex));
            free(threadPool);
        }
    }
}


/**
 * the function takes a task from the task-queue and executes it with the assistance of threads from the thread pool.
 * @param tp - the thread pool
 */
void* execute(void* tp) {
    Task* currentTask;
    //create the thread pool and the queue of tasks
    ThreadPool *threadPool = (ThreadPool*)tp;
    struct os_queue *queueOfTasks = threadPool->queueOfTasks;
    //remove tasks from the queue and execute them
    while (!((threadPool->availableToInsertion == FALSE) &&
             (osIsQueueEmpty(queueOfTasks))) && !(threadPool->isStop)) {
        pthread_mutex_lock(&(threadPool->queueLockMutex));
        if((!threadPool->isStop) && (osIsQueueEmpty(queueOfTasks))) {
            pthread_cond_wait(&(threadPool->condition), &(threadPool->queueLockMutex));
        }
        pthread_mutex_unlock(&(threadPool->queueLockMutex));

        pthread_mutex_lock(&(threadPool->lockMutex));
        if (!(osIsQueueEmpty(queueOfTasks))) {
            // remove task from the queue and execute it
            currentTask = osDequeue(queueOfTasks);
            pthread_mutex_unlock(&(threadPool->lockMutex));
            currentTask->function(currentTask->parameters);
            free(currentTask);
        }
        else {
            pthread_mutex_unlock(&(threadPool->lockMutex));
        }
    }
}


/**
 * the function prints error message in the error channel
 */
void failure() {
    write(STDERR_CHANNEL, "Error in system call\n", 20);
}

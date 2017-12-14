/*
 * file:        qthread.h
 * description: assignment - simple emulation of POSIX threads
 * class:       CS 5600, Fall 2017
 */
#ifndef __QTHREAD_H__
#define __QTHREAD_H__

#include <sys/socket.h>

/* you need to define the following (typedef) types:
 *  qthread_t, qthread_mutex_t, qthread_cond_t
 * I'll assume you're defining them as aliases for structures of the
 *  same name (minus the "_t") for mutex and cond, and a pointer to
 *  'struct qthread' for qthread_t.
 */

/* forward reference. Define the contents in qthread.c
 */
struct qthread; 
struct threadq; // for head and tail
typedef struct qthread *qthread_t;

/* defining the global variable STACK_SIZE(which is the size of the stack) 
 */
//extern const  int STACK_SIZE;
#define STACK_SIZE 8192

//first define the structure for qthread

struct qthread
{
    struct qthread *next; // next pointer
    void *sp; // stack pointer pointing to a particular stack
    void *stack; // stack which stores the stack stucture
    void *retval; // return value
    void *waiter; // waiter pointer to point to waiter of a thread
    int done; // done flag  ( for a thread saying its done suppose 1 for done and 2 for not done)  
    char flag; // a flag for weather reading or writing r for "reading" and w for "writing"
    int  fd; // a pointer to store which file descriptor we are waiting for
};


// strcuture thread which has head and tail
struct threadq{
    struct qthread *head,*tail;
};

// structure for qthread_mutex
struct qthread_mutex{
    int locked; // a flag to tell weather locked or not, 1 for locked and 0 for unlocked
    struct threadq waiters; // a threadq of waiters
};


// structure for condition variables
struct qthread_cond
{
    struct threadq waiters; // a threadq of waiters

};

typedef struct qthread_mutex qthread_mutex_t;
typedef struct qthread_cond qthread_cond_t;

/* prototypes - see qthread.c for function descriptions
 */

void qthread_run(void);
qthread_t qthread_start(void (*f)(void*, void*), void *arg1, void *arg2);
qthread_t qthread_create(void* (*f)(void*), void *arg1);
void qthread_helper(void* f, void *arg);
void qthread_yield(void);
void qthread_exit(void *val);
void *qthread_join(qthread_t thread);
void qthread_mutex_init(qthread_mutex_t *mutex);
void qthread_mutex_lock(qthread_mutex_t *mutex);
void qthread_mutex_unlock(qthread_mutex_t *mutex);
void qthread_cond_init(qthread_cond_t *cond);
void qthread_cond_wait(qthread_cond_t *cond, qthread_mutex_t *mutex);
void qthread_cond_signal(qthread_cond_t *cond);
void qthread_cond_broadcast(qthread_cond_t *cond);
unsigned get_usecs(void);
void qthread_usleep(long int usecs);
ssize_t qthread_read(int sockfd, void *buf, size_t len);
int qthread_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
ssize_t qthread_write(int sockfd, void *buf, size_t len);

#endif

/*
 * file:        qthread.c
 * description: assignment - simple emulation of POSIX threads
 * class:       CS 5600, Fall 2017
 */
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/select.h>
#include <errno.h>
#include "qthread.h"


/* prototypes for stack.c and switch.s
 * see source files for additional details
 */
extern void switch_to(void **location_for_old_sp, void *new_value);
extern void *setup_stack(int *stack, void *func, void *arg1, void *arg2);

void *main_sp;

// current,active, sleepers and io_waiters
struct qthread *current;
struct threadq active;
struct threadq sleepers;
struct threadq io_waiters;
struct qthread *current_io_waiter; // the pointer to the io_waiter in io_waiters list


// a function to check if the active list is empty
int tq_empty(struct threadq *tq){
    return tq->head==NULL;
}

// a pop function
struct qthread * tq_pop(struct threadq *tq){
    if(tq->head==NULL)
        return NULL;
    struct qthread *t=tq->head;
    tq->head=t->next;
    if(tq->head==NULL)
        tq->tail=NULL;
    return t;
}

//  an append function
void tq_append(struct threadq *tq , struct qthread *item){
    item->next=NULL;
    if(tq->head==NULL)
        tq->head=tq->tail=item;
    else {
        tq->tail->next=item;
        tq->tail=item;
    }

}

// IO_Wait function which wait for the waiters and then as we have waited we can add them back to active list
void IO_wait(struct threadq io_wait ){
    // for all the fd's 
    while(!tq_empty(&io_wait)){ 
        struct qthread *th= tq_pop(&io_wait);
        if(th->flag=='r'){ // checking if the fd is waiting for read (also accept)
            //set fd to rfds
            fd_set rfds; // setting rfds as this is read wait
            FD_ZERO(&rfds); // initializing it to empty set
            FD_SET(th->fd,&rfds);// setting the rfds
            // calling the select to wait for read
            select(FD_SETSIZE,&rfds,NULL,NULL,NULL);

        } else if (th->flag=='w'){ // checking if fd is waiting for write
            //set fd to wfds
            fd_set wfds; // setting wfds as this is write wait
            FD_ZERO(&wfds); // initializing it to empty set
            FD_SET(th->fd,&wfds);// setting the wfds
            // calling the select to wait for write
            select(FD_SETSIZE,NULL,&wfds,NULL,NULL);     
        } 

    }

}


// definition of qthread_create()
qthread_t qthread_create(void* (*f)(void*), void *arg){
    return qthread_start(qthread_helper, f, arg);
}

//definition of helper()
void qthread_helper(void* f, void *arg) {
    void* (*tmp)(void*) = f;
    qthread_exit(tmp(arg));
}


// definition to start a thread
qthread_t qthread_start(void (*f)(void*, void*), void *arg1, void *arg2){
    struct qthread *t =malloc(sizeof(*t));
    memset(t,0,sizeof(*t));

    t->stack=malloc(STACK_SIZE);
    t->sp=setup_stack(t->stack+STACK_SIZE,f,arg1,arg2);

    tq_append(&active,t);
    return t;
}

/* Schedules a new thread at the given save location
 */
void schedule(void *save_location)
{
    struct qthread *self=current;
again:
    current=tq_pop(&active);
    // if trying to switch to ourself then return
    if(current==self) {
        return;
    } if (current == NULL) {
        if (tq_empty(&sleepers)) {
            switch_to(NULL,main_sp);
        } else if (!tq_empty(&io_waiters)) {    
            // sending the waiter list to io_wait to wait for io
            IO_wait(io_waiters);
            // once we have returned from the io wait function , it means that we have waited for IO and now we can 
            // add them back to the active list ( add all of them as we have waited for IO for all of them)
            while(!tq_empty(&io_waiters)) {
                struct qthread *t1=tq_pop(&io_waiters);
                tq_append(&active,t1);
            }
        } else {
            usleep(20000);
            while(!(tq_empty(&sleepers))){
                struct qthread *t=tq_pop(&sleepers);
                tq_append(&active,t);
            }
            goto again;
        }
    }
    switch_to(save_location,current->sp);
}


// definition of qthread_run
void qthread_run(void){
    schedule(&main_sp);
}

/* qthread_exit, qthread_join - exit argument is returned by
 * qthread_join. Note that join blocks if thread hasn't exited yet,
 * and may crash if thread doesn't exist.
 */

// definition for qthread_yield
void qthread_yield(void){
    // struct qthread *self=current;
    tq_append(&active,current);
    // calling schedule function for the scheduling of threads
    schedule(&current->sp);
    //switch_to(&self->sp,current->sp);


}

// qthread_exit function declaration
void qthread_exit(void *val){
    struct qthread *self=current;
    current=tq_pop(&active);
    //setting the return value for the thread
    self->retval=val;
    if(self->waiter!=NULL) // i.e. waiter is present so wake it up by adding it to active list
        tq_append(&active,self->waiter);
    //switch_to(&current->sp, current->waiter->sp); //switching from current to waiter i.e. waking it up by putting it to ctive list
    //setting done to true
    self->done=1;

    if(current==NULL)
        switch_to(NULL,main_sp);
    else
        schedule(&self->sp);
}

// qthread_join function declaration
void *qthread_join(qthread_t thread){
    if (thread->done) {
        return thread->retval;
    }
    schedule(&thread->sp);
    struct qthread *self=current;
    thread->waiter=self; //the thread we are calling the join with waits for the thread to be done i.e. thread is waiter
    void* val=thread->retval;
    free(thread); // freeing the current thread by the waiting thread
    //returning the thread value
    return val;

}

// declaring a mutex variable
qthread_mutex_t m;

// definitions of qthread_mutex_init/lock/unlock
void qthread_mutex_init(qthread_mutex_t *mutex){
    mutex->locked=0; // initialized to unlocked
    mutex->waiters.head = (qthread_t) NULL;
    mutex->waiters.tail = (qthread_t) NULL;
}

void  qthread_mutex_lock(qthread_mutex_t *mutex){
    if(!(mutex->locked)) // if not locked lock it
        mutex->locked=1;
    else {
        tq_append(&mutex->waiters,current); // adding current to waiters as it has to wait   
        schedule(&current->sp);
    }
}

void qthread_mutex_unlock(qthread_mutex_t *mutex){
    // if no waiters then locked=0, else remove one of the waiters and put it to active list
    if(tq_empty(&mutex->waiters))
        mutex->locked=0;
    else
        tq_append(&active,tq_pop(&mutex->waiters));
}

// declaration of qthread_cond_init
void qthread_cond_init(qthread_cond_t *cond){
    cond->waiters.head = (qthread_t) NULL;
    cond->waiters.tail = (qthread_t) NULL;
}
// declaration of qthread_cond_wait
void qthread_cond_wait(qthread_cond_t *cond, qthread_mutex_t *mutex){
    // add the current to the waiters list
    tq_append(&cond->waiters,current);
    // unlocking the mutex
    qthread_mutex_unlock(mutex);
    // calling schedule
    schedule(&current->sp);
    // locking the mutex after going to sleep
    qthread_mutex_lock(mutex);
}

// declaration of qthread_cond_signal
void qthread_cond_signal(qthread_cond_t *cond){
    if (tq_empty(&cond->waiters)) {
        return;
    }
    // popping single element from the  waiter list and adding it to active
    tq_append(&active,tq_pop(&cond->waiters));

}

// declaration of void qthread_cond_broadcast
void qthread_cond_broadcast(qthread_cond_t *cond){
    // popping every element from the list
    while(!tq_empty(&cond->waiters)){
        tq_append(&active,tq_pop(&cond->waiters));
    }
}



/* POSIX replacement API. These are all the functions (well, the ones
 * used by the sample application) that might block.
 *
 * If there are no runnable threads, the scheduler needs to block
 * waiting for one of these blocking functions to return.
 *
 * Done using the select() system call, indicating all the
 * file descriptors that threads are blocked on, and with a timeout
 * for the earliest thread waiting in qthread_usleep()
 */

/* You'll need to tell time in order to implement qthread_usleep.
 * Here's an easy way to do it. 
 */
unsigned get_usecs(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec*1000000 + tv.tv_usec;
}

/* qthread_usleep - yield to next runnable thread, making arrangements
 * to be put back on the active list after 'usecs' timeout. 
 */
void qthread_usleep(long int usecs){
    unsigned wait_time=get_usecs()+usecs;
    while(get_usecs()<wait_time){
        tq_append(&sleepers,current);
        schedule(&current->sp);
    }
}

/* make sure that the file descriptor is in non-blocking mode, try to
 * read from it, if you get -1 / EAGAIN then add it to the list of
 * file descriptors to go in the big scheduling 'select()' and switch
 * to another thread.
 */
ssize_t qthread_io(ssize_t (*op)(int, void*, size_t), int fd, void *buf, size_t len)
{
    /* set non-blocking mode every time. If we added some more
     * wrappers we could set non-blocking mode from the beginning, but
     * this is a lot simpler (if less efficient). Do this for _write
     * and _accept, too.
     */
    int val, tmp = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, tmp | O_NONBLOCK);
}

/* like read - make sure the descriptor is in non-blocking mode, check
 * if if there's anything there - if so, return it, otherwise save fd
 * and switch to another thread. Note that accept() counts as a 'read'
 * for the select call.
 */

// definition of qthread_read()
ssize_t qthread_read(int fd, void *buf, size_t len){
    //  setting to non blocking
    int val, tmp = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, tmp | O_NONBLOCK);
    // getting the current thread
    struct qthread *self=current;
    // reading the file buffer
    val=read(fd,buf,len);
    if((val==-1) && (errno=EAGAIN)){
        // set the fd we are waiting for
        self->fd=fd;
        // set the flag to r indicating that we are reading
        self->flag='r';
        // put self to io_waiters and schedule self
        tq_append(&io_waiters,self);
        // call schedule with self
        schedule(&self->sp);

    }
    // returning the value
    return val;
}

/* Like read, again. Note that this is an output, rather than an input
 * - it can block if the network is slow, although it's not likely to
 * in most of our testing.
 */

//definition of accept
int qthread_accept(int fd, struct sockaddr *addr, socklen_t *addrlen){
    //  setting to non blocking
    int val, tmp = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, tmp | O_NONBLOCK);
    // getting the current thread
    struct qthread *self=current;
    // calling the accept function 
    val=accept(fd,addr,addrlen);
    if((val==-1) && (errno=EAGAIN)){
        // set the fd we are waiting for
        self->fd=fd;
        // set the flag to r( accept is a read function)
        self->flag='r';
        // put self to io_waiters and schedule self
        tq_append(&io_waiters,self);
        // call schedule with self
        schedule(&self->sp);

    }
    // returnig the value
    return val;
}

// definition of qthread_write()
ssize_t qthread_write(int fd, void *buf, size_t len){
    //  setting to non blocking
    int val, tmp = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, tmp | O_NONBLOCK);
    // getting the current thread
    struct qthread *self=current;
    // writing 
    val=write(fd,buf,len);
    if((val==-1) && (errno=EAGAIN)){
        // set the fd we are waiting for
        self->fd=fd;
        // set the flag to w indicating that we are writing
        self->flag='w';
        // put self to io_waiters and schedule self
        tq_append(&io_waiters,self);
        // call schedule with self
        schedule(&self->sp);

    }
    // returning the value
    return val;
}

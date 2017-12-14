#include <stdio.h>
#include <stdlib.h>

#include "qthread.h"

void testThreeThreads();
void testYieldMultipleTimes();
void testYieldBackForth();
void testYieldThreeThread();
void testJoinBeforeChildExit();
void testJoinAfterChildExit();
void testLockWhenUnlocked();
void testLockNoWaiting();
void testLockOneWaiting();
void testLockMultiWaiting();
void testCondWaitSignalZeroThreads();
void testCondThreeThreads();
void testCondWaitSignalOneThread();
void testCondWaitSignalTwoThreads();
void testSleepOneThread();
void testSleepTwoThreads();
void testSleepThreeThreads();
void testSleepTwoThreadsDiffTimeout();
void testSleepThreeThreadsDiffTimeout();

qthread_mutex_t m, m1;
qthread_cond_t c1,c2,c3;

int main(int argc, char** argv)
{
    /* one or more calls to qthread_start or qthread_create */
    /* then run */

    qthread_mutex_init(&m);
    qthread_mutex_init(&m1);
    qthread_cond_init(&c1);
    qthread_cond_init(&c2);
    qthread_cond_init(&c3);

    testThreeThreads();
    testYieldMultipleTimes();
    testYieldBackForth();
    testYieldThreeThread();
    testJoinBeforeChildExit();
    testJoinAfterChildExit();
    testLockWhenUnlocked();
    testLockNoWaiting();
    testLockOneWaiting();
    testLockMultiWaiting();
    testCondWaitSignalZeroThreads();
    testCondWaitSignalOneThread();
    testCondWaitSignalTwoThreads();
    testCondThreeThreads();
    testSleepOneThread();
    testSleepTwoThreads();
    testSleepThreeThreads();
    testSleepTwoThreadsDiffTimeout();
    testSleepThreeThreadsDiffTimeout();
    return 0;
}


void* printToTen(void *msg) {
    int i;

    //qthread_usleep(300000);
    //qthread_mutex_lock(&m);

    for (i = 0; i < 10; i++) {
        printf("%s %d \n", (char*) msg, i);
        qthread_usleep(100000);
        qthread_yield();
        qthread_yield();
        qthread_yield();
        qthread_yield();
    }
    //qthread_mutex_unlock(&m);
    qthread_exit(NULL);
}

void* printToTen0(void *msg) {
    int i;

    qthread_usleep(300000);
    //qthread_mutex_lock(&m);

    for (i = 0; i < 10; i++) {
        printf("%s %d \n", (char*) msg, i);
        //qthread_usleep(100000);
    }
    qthread_usleep(300000);
    //qthread_mutex_unlock(&m);
    qthread_exit(NULL);
}

void* printToTen1(void *msg) {
    int i;

    qthread_usleep(300000);
    //qthread_mutex_lock(&m);

    for (i = 0; i < 10; i++) {
        printf("%s %d \n", (char*) msg, i);
        qthread_usleep(100000);
    }
    qthread_usleep(300000);
    qthread_join(qthread_create(printToTen0, "Child1"));
    //qthread_mutex_unlock(&m);
    qthread_exit(NULL);
}

void* printToTen2(void *msg) {
    int i;

    qthread_usleep(300000);
    //qthread_mutex_lock(&m);

    for (i = 0; i < 10; i++) {
        printf("%s %d \n", (char*) msg, i);
        qthread_usleep(100000);
    }
    //qthread_mutex_unlock(&m);
    qthread_exit(NULL);
    qthread_usleep(100000);
    qthread_join(qthread_create(printToTen0, "Child2"));
    qthread_usleep(300000);
}

void* printToTen3(void *msg) {
    int i;
    printf("Locked: (Thread #%s) %d \n", (char*) msg,  m.locked);
    printf("Locking...\n");
    qthread_mutex_lock(&m);
    printf("Locked: %d \n", m.locked);
    qthread_usleep(300000);
    printf("Locking AGAIN...\n");
    qthread_mutex_lock(&m);
    qthread_usleep(300000);
    printf("Locked: %d \n", m.locked);	

    for (i = 0; i < 10; i++) {
        printf("%s %d \n", (char*) msg, i);
        qthread_usleep(100000);
    }
    printf("Unlocking mutex...\n");
    qthread_mutex_unlock(&m);
    qthread_usleep(300000);
    qthread_exit(NULL);
}

void* printToTen4(void *msg) {
    int i;
    qthread_usleep(300000);

    for (i = 0; i < 10; i++) {
        qthread_mutex_lock(&m);
        qthread_cond_wait(&c1, &m);
        printf("%s %d \n", (char*) msg, i);
        qthread_usleep(100000);
        qthread_cond_signal(&c1);
        qthread_mutex_unlock(&m);
    }
    qthread_usleep(300000);
    qthread_exit(NULL);
}

void testThreeThreads() {
    printf("Output of three threads\n");
    qthread_t t1 = qthread_create(printToTen0, "1");
    qthread_t t2 = qthread_create(printToTen0, "2");
    qthread_t t3 = qthread_create(printToTen0, "3");
    qthread_run();

    printf("\n");
}

void testYieldMultipleTimes() {
    printf("Testing 1 Thread, YIELD multiple, exit\n");
    qthread_t t1 = qthread_create(printToTen, "1");
    qthread_run();
    printf("\n");
}

void testYieldBackForth() {
    printf("Testing 2 Threads, YIELD back/forth, exit\n");
    qthread_t t1 = qthread_create(printToTen, "1");
    qthread_t t2 = qthread_create(printToTen, "2");
    qthread_run();
    printf("\n");
}

void testYieldThreeThread() {
    printf("Testing 3 Threads, YIELD back/forth, exit\n");		
    qthread_t t1 = qthread_create(printToTen, "1");
    qthread_t t2 = qthread_create(printToTen, "2");
    qthread_t t3 = qthread_create(printToTen, "3");
    qthread_run();
    printf("\n");
}

void testJoinBeforeChildExit() {
    printf("Testing join call before child thread exits\n");
    qthread_t t1 = qthread_create(printToTen1, "1");
    qthread_run();
    printf("\n");
}

void testJoinAfterChildExit() {
    printf("Testing join call after child thread exits\n");
    qthread_t t1 = qthread_create(printToTen2, "1");
    qthread_run();
    printf("\n");
}

void testLockWhenUnlocked() {
    printf("Testing mutex lock when unlocked\n");
    printf("Locked: %d \n", m.locked);
    printf("Locking mutex...\n");
    qthread_mutex_lock(&m);
    printf("Locked: %d \n", m.locked);
    printf("Running thread\n");
    qthread_t t1 = qthread_create(printToTen, "1");
    qthread_run();
    printf("Unlocking mutex...\n");
    qthread_mutex_unlock(&m);
    printf("Locked: %d \n", m.locked);
    printf("\n");
}

void testLockNoWaiting() {
    printf("Testing mutex lock when locked, 0 threads waiting\n");
    qthread_run();
    printf("\n");
}

void testLockOneWaiting() {
    printf("Testing mutex lock when locked, 1 thread waiting\n");
    qthread_t t1 = qthread_create(printToTen3, "1");
    qthread_run();
    printf("\n");
}

void testLockMultiWaiting() {
    printf("Testing mutex lock when locked, 2 threads waiting\n");
    qthread_t t1 = qthread_create(printToTen3, "1");
    qthread_t t2 = qthread_create(printToTen3, "2");
    qthread_run();
    printf("\n");
}

void* printCond1(void *msg) {
    int i;

    qthread_usleep(50000);
    qthread_mutex_lock(&m1);

    for (i = 0; i < 10; i++) {
        if (i !=  0) { 
            qthread_cond_wait(&c1, &m1);
        }
        printf("%s: %d \n", (char*) msg, i);
        qthread_usleep(20000);
        qthread_cond_signal(&c2);
    }
    qthread_mutex_unlock(&m1);
    qthread_usleep(50000);
    qthread_exit(NULL);
}

void* printCond2(void *msg) {
    int i;

    qthread_usleep(50000);
    qthread_mutex_lock(&m1);

    for (i = 0; i < 10; i+=2) {	
        qthread_cond_wait(&c2, &m1);
        printf("%s: %d \n", (char*) msg, i);
        qthread_usleep(50000);
        qthread_cond_signal(&c3);
    }
    qthread_mutex_unlock(&m1);
    qthread_usleep(50000);
    qthread_exit(NULL);
}

void* printCond3(void *msg) {
    int i;

    qthread_usleep(50000);
    qthread_mutex_lock(&m1);

    for (i = 0; i < 10; i++) {
        qthread_cond_wait(&c3, &m1);
        printf("%s: %d \n", (char*) msg, i);
        qthread_usleep(50000);
        qthread_cond_signal(&c2);
    }
    qthread_mutex_unlock(&m1);
    qthread_usleep(50000);
    qthread_exit(NULL);
}

void testCondWaitSignalZeroThreads() {
    printf("Testing cond var when 0 threads already waiting\n");
    qthread_run();
    printf("\n");
}

void testCondThreeThreads() {
    printf("Testing cond var with 3 threads\n");
    qthread_t t1 = qthread_create(printCond1, "1");
    qthread_t t2 = qthread_create(printCond2, "2");
    qthread_t t3 = qthread_create(printCond3, "3");

    qthread_run();

    qthread_join(t3);
    qthread_join(t2);
    qthread_join(t1);
    printf("\n");
}

void testCondWaitSignalOneThread() {
    printf("Testing vond var when 1 thread waiting\n");
    qthread_t t1 = qthread_create(printCond1, "1");
    qthread_run();
    printf("\n");
}

void testCondWaitSignalTwoThreads() {
    printf("Testing cond var when 2 threads already waiting\n");
    qthread_t t1 = qthread_create(printCond1, "1");
    qthread_t t2 = qthread_create(printCond2, "2");
    qthread_run();
    printf("\n");
}

void* printSleep(void* msg) {
    printf("%s - before sleep for 100 ms\n", (char*) msg);
    int beforeTime = get_usecs();
    qthread_usleep(100000);
    int afterTime = get_usecs();
    printf("%s - after sleep for 100 ms\n", (char*) msg);
    printf("%s - difference in time >> %d\n", (char*) msg, afterTime - beforeTime);
    qthread_exit(NULL);
}

void* printSleep2(void* msg) {
    printf("%s - before sleep for 500 ms\n", (char*) msg);
    int beforeTime = get_usecs();
    qthread_usleep(500000);
    int afterTime = get_usecs();
    printf("%s - after sleep for 500 ms\n", (char*) msg);
    printf("%s - difference in time >> %d\n", (char*) msg, afterTime - beforeTime);
    qthread_exit(NULL);
}
void testSleepOneThread() {
    printf("Testing sleep with 1 thread with same timeout\n");
    qthread_t t1 = qthread_create(printSleep, "1");
    qthread_run();
    printf("\n");
}

void testSleepTwoThreads() {
    printf("Testing sleep with 2 threads with same timeout\n");
    qthread_t t1 = qthread_create(printSleep, "1");
    qthread_t t2 = qthread_create(printSleep, "2");
    qthread_run();
    printf("\n");
}

void testSleepThreeThreads() {
    printf("Testing sleep with 3 threads with same timeout\n");
    qthread_t t1 = qthread_create(printSleep, "1");
    qthread_t t2 = qthread_create(printSleep, "2");
    qthread_t t3 = qthread_create(printSleep, "3");
    qthread_run();
    printf("\n");
}

void testSleepTwoThreadsDiffTimeout() {
    printf("Testing sleep with 2 threads with variable timeouts\n");
    qthread_t t1 = qthread_create(printSleep, "1");
    qthread_t t2 = qthread_create(printSleep2, "2");
    qthread_run();
    printf("\n");
}

void testSleepThreeThreadsDiffTimeout() {
    printf("Testing sleep with 3 threads with variable timeouts\n");
    qthread_t t1 = qthread_create(printSleep2, "1");
    qthread_t t2 = qthread_create(printSleep, "2");
    qthread_t t3 = qthread_create(printSleep2, "3");
    qthread_run();
    printf("\n");
}

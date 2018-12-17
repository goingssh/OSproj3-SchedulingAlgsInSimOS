/*
 * sched.h
 * CPU scheduler for the simulation 
 *   
 * Starting core code from http://www.cc.gatech.edu/~rama/CS2200-External
 * Code added/modified by Sherri Goings for CS332 Operating Systems Proj3
 * Last modified 2/3/2016
 * 
 * YOU WILL NOT NEED TO MODIFY THIS FILE
 */


#ifndef __SCHED_H__
#define __SCHED_H__

#include "simOS.h"

/* Functions called from simulator - comments in sched.c */
extern void idle(unsigned int cpu_id);
extern void preempt(unsigned int cpu_id);
extern void yield(unsigned int cpu_id);
extern void terminate(unsigned int cpu_id);
extern void wake_up(pcb_t *process);


/*
 * current[] is an array representing the simulated CPU's.
 * There is one array element CORRESPONDING TO EACH CPU in the simulation.
 * Each array element is a pointer to the processe currently running on that CPU.
 * e.g. if you're simulating on 2 CPU's, current[0] is the process running on
 * the first CPU, and current[1] is the process running on the second.
 *
 * current[] should be updated by schedule() each time a process is scheduled
 * on a CPU.  Since the current[] array is accessed by multiple threads, you
 * will need to use a mutex to protect it.  current_mutex has been provided
 * for your use.
 */
static pcb_t **current;
static pthread_mutex_t current_mutex;

// head and tail of ready queue
static pcb_t* head = NULL;
static pcb_t* tail = NULL;

// mutex to protect ready queue
static pthread_mutex_t ready_mutex;

// cond var for idle() to sleep on until a process is available on the ready queue
static pthread_cond_t ready_empty;

#endif /* __SCHED_H__ */

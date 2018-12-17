/*
 * sched.c
 * CPU scheduler for the simulation - completed solution.
 *   
 * Starting core code from http://www.cc.gatech.edu/~rama/CS2200-External
 * Code added/modified by Sherri Goings for CS332 Operating Systems Proj3
 * Last modified 12/15/2018
 */

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "simOS.h"
#include "sched.h"

// Local helper functions
static void schedule(unsigned int cpu_id);
static void addReadyProcess(pcb_t* proc); 
static pcb_t* getReadyProcess(void); 

/*
 * possible scheduling algorithms
 */
typedef enum {
    FIFO = 0,
    RoundRobin,
    StaticPriority,
    MultiLevelFeedback
} scheduler_alg;

scheduler_alg alg;

// declare other global vars
int time_slice = -1;
int cpu_count;

// Queues for multi-level feedback
pcb_t* multi_level_heads[4] = {NULL};
pcb_t* multi_level_tails[4] = {NULL};

/*
 * main() parses command line arguments, initializes globals, and starts simulation
 */
int main(int argc, char *argv[])
{
    /* Parse command line args - must include num_cpus as first, rest optional
     * Default is to simulate using just FIFO on given num cpus, if 2nd arg given:
     * if -r, use round robin to schedule, 3rd argument must also be included to 
	 * set timeslice)
     * if -p, use static priority to schedule
     * Extra Credit: if -m, use multi-level feedback queues, again 3rd arg for timeslice
     */
    if (argc == 2) {
        alg = FIFO;
        printf("running with basic FIFO\n");
    }
    else if (argc > 2 && strcmp(argv[2],"-r")==0 && argc > 3) {
        alg = RoundRobin;
        time_slice = atoi(argv[3]);
        printf("running with round robin, time slice = %d\n", time_slice);
    }
    else if (argc > 2 && strcmp(argv[2],"-p")==0) {
        alg = StaticPriority;
        printf("running with static priority\n");
    }
  else if (argc > 2 && strcmp(argv[2],"-m")==0 && argc > 3) {
    alg = MultiLevelFeedback;
    time_slice = atoi(argv[3]);
    printf("running with multi-level feedback, time slice = %d\n", time_slice);
  }
    else {
        fprintf(stderr, "Usage: ./simOS <# CPUs> [ -r <time slice> | -m <time slice> | -p ]\n"
            "    Default : FIFO Scheduler\n"
            "         -r : Round-Robin Scheduler (must also give time slice)\n"
            "         -m : Multi-Level Feedback Scheduler (must also give time slice)\n"
            "         -p : Static Priority Scheduler\n\n");
        return -1;
    }
    fflush(stdout);

    /* atoi converts string to integer */
    cpu_count = atoi(argv[1]);

    /* Allocate the current[] array of cpus and its mutex */
    current = malloc(sizeof(pcb_t*) * cpu_count);
    int i;
    for (i=0; i<cpu_count; i++) {
        current[i] = NULL;
    }
    assert(current != NULL);
    pthread_mutex_init(&current_mutex, NULL);

    /* Initialize other necessary synch constructs */
    pthread_mutex_init(&ready_mutex, NULL);
    pthread_cond_init(&ready_empty, NULL);

    /* Start the simulator in the library */
    printf("starting simulator\n");
    fflush(stdout);
    start_simulator(cpu_count);


    return 0;
}

/* 
 * returns whether the CPU should idle
 * need slightly more complicated check than simply head==null
 * in order to handle multilevel feedback implementation.
 */
int should_idle() {
  // Check head (used for everything except multi-level)
  int no_procs = head == NULL;

  if (alg == MultiLevelFeedback) {
    // If multi-level, check all the queues
    for (int i = 3; i >= 0; i--) {
      if (multi_level_heads[i] != NULL) {
        // Stop idling if any processes in queue!
        no_procs = 0;
        break;
      }
    }
  }
  return no_procs;
}

/*
 * idle() is called by the simulator when the idle process is scheduled.
 * It blocks until a process is added to the ready queue, and then calls
 * schedule() to select the next process to run on the CPU.
 */
extern void idle(unsigned int cpu_id)
{
  pthread_mutex_lock(&ready_mutex);

  // Idle while there are no processes ready to run
  while (should_idle()) {
    pthread_cond_wait(&ready_empty, &ready_mutex);
  }

  pthread_mutex_unlock(&ready_mutex);
  schedule(cpu_id);
}

/*
 * schedule() is your CPU scheduler. It currently implements basic FIFO scheduling -
 * 1. calls getReadyProcess to select and remove a runnable process from your ready queue
 * 2. updates the current array of CPU's to show this process (or NULL if there was none) 
 *    as running on the given cpu 
 * 3. sets this process state to running (unless its the NULL process)
 * 4. calls context_switch to actually start the chosen process on the given cpu
 *    - note if proc==NULL the idle process will be run
 *    - note the final arg of -1 means there is no clock interrupt
 *  context_switch() is prototyped in simOS.h. Look there for more information. 
 *  a basic getReadyProcess() is implemented below, look at the comments for info.
 */
static void schedule(unsigned int cpu_id) {
    pcb_t* proc = getReadyProcess();

    pthread_mutex_lock(&current_mutex);
    current[cpu_id] = proc;
    pthread_mutex_unlock(&current_mutex);

    if (proc!=NULL) {
        proc->state = PROCESS_RUNNING;
    }

    context_switch(cpu_id, proc, time_slice); 
}


/*
 * preempt() is the handler called by the simulator when a process is
 * preempted due to its timeslice expiring.
 * cpu_id is the index of THIS CPU in the "current" array of cpu's.
 *
 * This function should get the process currently running on the given cpu, 
 * change the process state to ready, place the process back in the
 * ready queue (for FIFO just use addReadyProcess), and finally call 
 * schedule() for this cpu to select a new runnable process.
 *
 */
extern void preempt(unsigned int cpu_id) {
  pthread_mutex_lock(&current_mutex);
  current[cpu_id]->state = PROCESS_READY;

  // If MLF scheduler, also decrement priority if possible
  if (alg == MultiLevelFeedback && current[cpu_id]->temp_priority > 0) {
    current[cpu_id]->temp_priority--;
  }

  pthread_mutex_unlock(&current_mutex);
  addReadyProcess(current[cpu_id]);
  schedule(cpu_id);
}


/*
 * yield() is called by the simulator when a process performs an I/O request 
 * note this is different than the concept of yield in user-level threads!
 * In this context, yield sets the state of the process to waiting (on I/O),
 * then calls schedule() to select a new process to run on this CPU.
 * args: int - id of CPU process wishing to yield is currently running on.
 */
extern void yield(unsigned int cpu_id) {
    // use lock to ensure thread-safe access to current process
    pthread_mutex_lock(&current_mutex);
    current[cpu_id]->state = PROCESS_WAITING;

    // If MLF scheduler, also increment priority if possible
    if (alg == MultiLevelFeedback && current[cpu_id]->temp_priority < 3) {
      current[cpu_id]->temp_priority++;
    }

    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}


/*
 * terminate() is called by the simulator when a process completes.
 * marks the process as terminated, then calls schedule() to select
 * a new process to run on this CPU.
 * args: int - id of CPU process wishing to terminate is currently running on.
 */
extern void terminate(unsigned int cpu_id) {
    // use lock to ensure thread-safe access to current process
    pthread_mutex_lock(&current_mutex);
    current[cpu_id]->state = PROCESS_TERMINATED;
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}

/*
 * wake_up() is called for a new process and when an I/O request completes.
 * For all 3 scheduling algorithms, mark process as READY and call
 * addReadyProcess where the appropriate queue will updated depending on
 * the scheduler being used.
 * 
 * FIFO and MLF scheduling are both non-preemptive, so nothing else to do.
 *
 * Static priority scheduling IS preemptive, so if all processes are in use,
 * and lowest priority process currently running is lower than process just
 * woken up, preempt CPU with that lowest priority process. The force_preempt
 * function will actually handle calling preempt to schedule the next process.
 */
extern void wake_up(pcb_t *process) {
  // If MLF scheduler and this is a new process, start it with highest priority
  if (alg == MultiLevelFeedback && process->state == PROCESS_NEW) {
    process->temp_priority = 3;
  }

  process->state = PROCESS_READY;
  addReadyProcess(process);

  if (alg == StaticPriority) {
    pthread_mutex_lock(&current_mutex);

	//** FIX **
    unsigned int lowest_priority = 11;
    int lowest_priority_index = -1;

    for (int i = 0; i < cpu_count; i++) {
      pcb_t* curr = current[i];

      if (curr == NULL) {
        // If any CPU is idling, don't preempt!
        pthread_mutex_unlock(&current_mutex);
        return;
      }

      if (curr->static_priority < lowest_priority) {
        // Find the process with the lowest priority and replace it
        lowest_priority = curr->static_priority;
        lowest_priority_index = i;
      }
    }

    pthread_mutex_unlock(&current_mutex);

    if (lowest_priority < process->static_priority) {
      // If current process' priority is higher than that of the lowest priority process,
      // preempt it!
      force_preempt(lowest_priority_index);
    }
  }
}


/* The following 2 functions implement a FIFO ready queue of processes */

/* 
 * addReadyProcess adds a process to the end of a pseudo linked list (each process
 * struct contains a pointer next that you can use to chain them together)
 * it takes a pointer to a process as an argument and has no return.
 * If using FIFO or static priority scheduling, only one linked list is maintained.
 * If using MLF, maintain 4 linked lists, 1 for each possible priority of a process,
 * and ready process is added to list that corresponds to its current priority.
 */
static void addReadyProcess(pcb_t* proc) {
  // ensure no other process can access ready list while we update it
  pthread_mutex_lock(&ready_mutex);

  // for MLF need 4 
  pcb_t** curr_head = &head;
  pcb_t** curr_tail = &tail;

  if (alg == MultiLevelFeedback) {
    // Use the appropriate priority queue
    curr_head = &multi_level_heads[proc->temp_priority];
    curr_tail = &multi_level_tails[proc->temp_priority];
  }

  // add this process to the end of the ready list
  if (*curr_head == NULL) {
    *curr_head = proc;
    *curr_tail = proc;
    // if list was empty may need to wake up idle process
    pthread_cond_signal(&ready_empty);
  }
  else {
    (*curr_tail)->next = proc;
    *curr_tail = proc;
  }

  // ensure that this proc points to NULL
  proc->next = NULL;

  pthread_mutex_unlock(&ready_mutex);
}


/* 
 * getReadyProcess gets the next process that should be run, depending on the scheduler.
 * all schedulers use their specific method to choose the next process to run, remove it 
 * from the ready queue(s), and return it.
 * takes no arguments, returns NULL if no processes are in ready state.
 * Static priority - chooses by searching single queue for highest priority process
 * MLF - chooses first process in highest priority non-empty queue of the 4 queues.
 * FIFO - chooses first process in the single queue.
 */
static pcb_t* getReadyProcess(void) {
  // ensure no other process can access ready list while we update it
  pthread_mutex_lock(&ready_mutex);

  if (alg == StaticPriority) {
    // if list is empty, unlock and return null
    if (head == NULL) {
      pthread_mutex_unlock(&ready_mutex);
      return NULL;
    }

    pcb_t* curr = head;

    unsigned int highest_priority = curr->static_priority;
    pcb_t* highest_priority_process = curr;

    // Find the process with the highest priority
    while (curr != NULL) {
      if (curr->static_priority > highest_priority) {
        highest_priority = curr->static_priority;
        highest_priority_process = curr;
      }
      curr = curr->next;
    }

	// ** FIX **
    // If the first process has the highest priority, update head!
    if (head == highest_priority_process) {
      head = highest_priority_process->next;
    }
    else {
      pcb_t* prev = head;
      curr = head->next;

      while (curr != NULL && prev != NULL) {
        if (curr == highest_priority_process) {
          // Update linked list
          prev->next = curr->next;

          if (curr == tail) {
            // Update tail
            tail = prev;
          }

          break;
        }

        prev = curr;
        curr = curr->next;
      }
    }

    if (head == NULL) {
      // Update tail
      tail = NULL;
    }

    pthread_mutex_unlock(&ready_mutex);
    return highest_priority_process;
  }
  // MLF or FIFO
  else {
    pcb_t** curr_head = &head;
    pcb_t** curr_tail = &tail;

	// for MFL, find highest priority queue that is not empty,
	// or arbitrarily choose last (lowest priority) if all empty.
    if (alg == MultiLevelFeedback) {
      for (int i = 3; i >= 0; i--) {
        curr_head = &multi_level_heads[i];
        curr_tail = &multi_level_tails[i];

        // If this queue isn't empty, it's the one, don't go on!
        if (*curr_head != NULL) {
          break;
        }
      }
    }

    // if queue is empty, unlock and return null
    if (*curr_head == NULL) {
      pthread_mutex_unlock(&ready_mutex);
      return NULL;
    }

    // otherwise, remove first process from queue and return.
    pcb_t* first = *curr_head;
    *curr_head = first->next;

    // if there was no next process, list is now empty, set tail to NULL
    if (*curr_head == NULL) *curr_tail = NULL;

    pthread_mutex_unlock(&ready_mutex);
    return first;
  }
}


/*
 * proc.c 
 * Multithreaded OS Simulation
 * Starting core code from http://www.cc.gatech.edu/~rama/CS2200-External
 * Code added/modified by Sherri Goings for CS332 Operating Systems Proj3
 * Last modified 2/3/2016
 *
 * This file contains process data for the simulator.
 * Note: should really change this to read process data from a file
 * to make testing more efficient!
 */

#include "simOS.h"
#include "proc.h"

/*
 * Each process is simply made up of alternating states a process will be in
 * (either using the CPU or waiting on IO request) and the # simulation steps 
 * each state lasts.
 *
 * Note: The operations must alternate: OP_CPU, OP_IO, OP_CPU, ...
 * In addition, the first and last non-termination operations must be OP_CPU.  
 * Otherwise, the simulator will not work.
 */

static op_t pid0_ops[] = {
    { OP_CPU, 2 },
    { OP_IO, 2 },
    { OP_CPU, 3 },
    { OP_IO, 5 },
    { OP_CPU, 1 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_IO, 2 },
    { OP_CPU, 3 },
    { OP_IO, 5 },
    { OP_CPU, 1 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_IO, 2 },
    { OP_CPU, 3 },
    { OP_IO, 5 },
    { OP_CPU, 1 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_IO, 5 },
    { OP_CPU, 1 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_IO, 2 },
    { OP_CPU, 3 },
    { OP_IO, 5 },
    { OP_CPU, 1 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_TERMINATE, 0 }
};

static op_t pid1_ops[] = {
    { OP_CPU, 3 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_IO, 6 },
    { OP_CPU, 1 },
    { OP_IO, 3 },
    { OP_CPU, 4 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_IO, 6 },
    { OP_CPU, 1 },
    { OP_IO, 3 },
    { OP_CPU, 4 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_IO, 6 },
    { OP_CPU, 1 },
    { OP_IO, 3 },
    { OP_CPU, 4 },
    { OP_IO, 3 },
    { OP_CPU, 4 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_IO, 6 },
    { OP_CPU, 1 },
    { OP_IO, 3 },
    { OP_CPU, 4 },
    { OP_TERMINATE, 0 }
};

static op_t pid2_ops[] = {
    { OP_CPU, 1 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_IO, 5 },
    { OP_CPU, 1 },
    { OP_IO, 3 },
    { OP_CPU, 3 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_IO, 5 },
    { OP_CPU, 1 },
    { OP_IO, 3 },
    { OP_CPU, 3 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_IO, 5 },
    { OP_CPU, 1 },
    { OP_IO, 3 },
    { OP_CPU, 3 },
    { OP_IO, 4 },
    { OP_CPU, 2 },
    { OP_IO, 5 },
    { OP_CPU, 1 },
    { OP_IO, 3 },
    { OP_CPU, 3 },
    { OP_TERMINATE, 0 }
};

static op_t pid3_ops[] = {
    { OP_CPU, 9 },
    { OP_IO, 1 },
    { OP_CPU, 6 },
    { OP_IO, 1 },
    { OP_CPU, 8 },
    { OP_IO, 1 },
    { OP_CPU, 7 },
    { OP_IO, 1 },
    { OP_CPU, 6 },
    { OP_IO, 1 },
    { OP_CPU, 8 },
    { OP_IO, 1 },
    { OP_CPU, 7 },
    { OP_IO, 1 },
    { OP_CPU, 6 },
    { OP_IO, 1 },
    { OP_CPU, 8 },
    { OP_IO, 1 },
    { OP_CPU, 8 },
    { OP_TERMINATE, 0 }
};

static op_t pid4_ops[] = {
    { OP_CPU, 10 }, 
    { OP_IO, 1 },
    { OP_CPU, 14 },
    { OP_IO, 1 },
    { OP_CPU, 7 },
    { OP_IO, 2 },
    { OP_CPU, 11 },
    { OP_IO, 1 },
    { OP_CPU, 14 },
    { OP_IO, 1 },
    { OP_CPU, 7 },
    { OP_IO, 2 },
    { OP_CPU, 11 },
    { OP_IO, 1 },
    { OP_CPU, 14 },
    { OP_IO, 1 },
    { OP_CPU, 7 },
    { OP_IO, 2 },
    { OP_CPU, 11 },
    { OP_TERMINATE, 0 }
};

static op_t pid5_ops[] = {
    { OP_CPU, 9 }, 
    { OP_IO, 1 },
    { OP_CPU, 10 },
    { OP_IO, 2 },
    { OP_CPU, 15 },
    { OP_IO, 1 },
    { OP_CPU, 8 },
    { OP_IO, 1 },
    { OP_CPU, 10 },
    { OP_IO, 2 },
    { OP_CPU, 15 },
    { OP_IO, 1 },
    { OP_CPU, 8 },
    { OP_IO, 1 },
    { OP_CPU, 10 },
    { OP_IO, 2 },
    { OP_CPU, 15 },
    { OP_IO, 1 },
    { OP_CPU, 8 },
    { OP_TERMINATE, 0 }
};

static op_t pid6_ops[] = {
    { OP_CPU, 6 }, 
    { OP_IO, 3 },
    { OP_CPU, 9 },
    { OP_IO, 1 },
    { OP_CPU, 14 },
    { OP_IO, 1 },
    { OP_CPU, 11 },
    { OP_IO, 3 },
    { OP_CPU, 9 },
    { OP_IO, 1 },
    { OP_CPU, 14 },
    { OP_IO, 1 },
    { OP_CPU, 11 },
    { OP_IO, 3 },
    { OP_CPU, 9 },
    { OP_IO, 1 },
    { OP_CPU, 14 },
    { OP_IO, 1 },
    { OP_CPU, 11 },
    { OP_TERMINATE, 0 }
};

static op_t pid7_ops[] = {
    { OP_CPU, 6 }, 
    { OP_IO, 3 },
    { OP_CPU, 12 },
    { OP_IO, 3 },
    { OP_CPU, 7 },
    { OP_IO, 1 },
    { OP_CPU, 9 },
    { OP_IO, 3 },
    { OP_CPU, 12 },
    { OP_IO, 3 },
    { OP_CPU, 7 },
    { OP_IO, 1 },
    { OP_CPU, 9 },
    { OP_IO, 3 },
    { OP_CPU, 12 },
    { OP_IO, 3 },
    { OP_CPU, 7 },
    { OP_IO, 1 },
    { OP_CPU, 9 },
    { OP_TERMINATE, 0 }
};

/*
 * put the 8 test processes in an array of process control blocks.
 * pcb_t struct defined in simOS.h with full description 
 * basic version: { pid, name, priority, cur state, array of operations }
 */
pcb_t processes[PROCESS_COUNT] = {
    { 0, "Iapache", 8, PROCESS_NEW, pid0_ops },
    { 1, "Ibash", 7, PROCESS_NEW, pid1_ops },
    { 2, "Imozilla", 7, PROCESS_NEW, pid2_ops },
    { 3, "Ccpu", 5, PROCESS_NEW, pid3_ops },
    { 4, "Cgcc", 1, PROCESS_NEW, pid4_ops },
    { 5, "Cspice", 2, PROCESS_NEW, pid5_ops },
    { 6, "Cmysql", 4, PROCESS_NEW, pid6_ops },
    { 7, "Csim", 3, PROCESS_NEW, pid7_ops }
};



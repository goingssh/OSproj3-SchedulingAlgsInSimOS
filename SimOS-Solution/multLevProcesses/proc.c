/*
 * proc.c 
 * Multithreaded OS Simulation
 * Starting core code from http://www.cc.gatech.edu/~rama/CS2200-External
 * Code added/modified by Sherri Goings for CS332 Operating Systems Proj3
 * Last modified 2/3/2016
 *
 * This file contains process data intended to test a multi-level feedback scheduler.
 * Replace proc.h and proc.c with these files to run the processes here.
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
    { OP_CPU, 100 },
    { OP_TERMINATE, 0 }
};

static op_t pid2_ops[] = {
    { OP_CPU, 100 },
    { OP_TERMINATE, 0 }
};

static op_t pid3_ops[] = {
    { OP_CPU, 100 },
    { OP_TERMINATE, 0 }
};

static op_t pid1_ops[] = {
    { OP_CPU, 2 },
    { OP_IO, 15 },
    { OP_CPU, 2 },
    { OP_IO, 15 },
    { OP_CPU, 2 },
	{ OP_IO, 15 },
    { OP_CPU, 2 },
	{ OP_IO, 15 },
    { OP_CPU, 2 },
	{ OP_IO, 15 },
    { OP_CPU, 2 },
    { OP_IO, 15 },
    { OP_CPU, 2 },
	{ OP_IO, 15 },
    { OP_CPU, 2 },
	{ OP_IO, 15 },
    { OP_CPU, 2 },
	{ OP_IO, 15 },
    { OP_CPU, 2 },
	{ OP_IO, 15 },
    { OP_CPU, 2 },
	{ OP_IO, 15 },
    { OP_CPU, 2 },
	{ OP_IO, 15 },
    { OP_CPU, 2 },
	{ OP_IO, 15 },
    { OP_CPU, 2 },
	{ OP_TERMINATE, 0 }
};

/*
 * put the 4 test processes in an array of process control blocks.
 * pcb_t struct defined in simOS.h with full description 
 * basic version: { pid, name, static priority, cur priority, cur state, array of operations }
 */
pcb_t processes[PROCESS_COUNT] = {
	{ 0, "cpu1", 5, 5, PROCESS_NEW, pid0_ops },
	{ 1, "IO", 5, 5, PROCESS_NEW, pid1_ops },
	{ 2, "cpu2", 5, 5, PROCESS_NEW, pid2_ops },
	{ 3, "cpu3", 5, 5, PROCESS_NEW, pid3_ops }
};



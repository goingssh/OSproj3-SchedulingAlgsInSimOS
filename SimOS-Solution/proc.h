/*
 * proc.h
 * Multithreaded OS Simulation
 * Starting core code from http://www.cc.gatech.edu/~rama/CS2200-External
 * Code added/modified by Sherri Goings for CS332 Operating Systems Proj3
 * Last modified 2/3/2016
 *
 * This file contains process data for the simulator.
 * Note: should really change this to read process data from a file
 * to make testing more efficient!
 */

#ifndef __PROC_H__
#define __PROC_H__

/* 
 * array of process control blocks, currently hard-coded at 8
 */
#define PROCESS_COUNT 8
extern pcb_t processes[PROCESS_COUNT];


#endif /* __PROC_H__ */


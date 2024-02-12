/*
 * k_task.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#ifndef INC_K_TASK_H_
#define INC_K_TASK_H_

#include <common.h>

extern int kernelInitalized;
extern int kernelRunning;
extern int taskExiting;

int osCreateTask(TCB* task);
void osKernelInit(void);
int osTaskExit(void);
int osKernelStart(void);
void osYield (void);
int osTaskInfo(task_t TID, TCB *task_copy);
int osTaskExit(void);

int CheckStackArr(int size);
void SetStackArr(int start, int length, int val);
void RunNullTask();


#endif /* INC_K_TASK_H_ */

/*
 * common.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: If you feel that there are common
 *      C functions corresponding to this
 *      header, then any C functions you write must go into a corresponding c file that you create in the Core->Src folder
 */

#include <stdint.h>

#ifndef INC_COMMON_H_
#define INC_COMMON_H_


#define TID_NULL 0 //predefined Task ID for the NULL task
#define MAX_TASKS 16 //maximum number of tasks in the system (ask if i need to subtract one for null task)
#define STACK_SIZE 0x200 //min. size of each task’s stack
#define MAIN_STACK_SIZE 0x400 //OS stack size
#define STACK_TOTAL_SIZE 0x4000

#define DORMANT 0 //state of terminated task
#define READY 1 //state of task that can be scheduled but is not running
#define RUNNING 2 //state of running task

#define RTX_ERR 0
#define RTX_OK 1
typedef uint32_t task_t;


typedef struct task_control_block{
  void (*ptask)(void* args); //entry address
  uint32_t stack_high; //start starting address (high address)
  uint32_t stack_ptr;
  task_t tid; //task ID (holds stack index starting 0)
  uint8_t state; //task's state
  uint16_t stack_size; //stack size. Must be a multiple of 8
  uint32_t startBlockIndex;
}TCB;


struct TcbQ {
  TCB queue[MAX_TASKS];
  int front;
  int rear;
  int size;
};

struct TidQ {
  int front, rear, size;
  int array[MAX_TASKS];
};


extern int totalEntries; //os needs a entry for kernel
extern char stackEntries[(STACK_TOTAL_SIZE-MAIN_STACK_SIZE)/STACK_SIZE]; //should all be set to 1 (available)

extern struct TcbQ tcbQ;
extern struct TidQ tidQ;
extern TCB runningTask;

extern uint32_t* MSP_INIT_VAL; //ptr to start of stack (not actually zero, references big address number)


uint32_t* getTaskStartAddress(int index);
void initTcbQ();
int isTcbQEmpty();
int isTcbQFull();
void pushTcbQ(TCB* tc); //copy by value
TCB popTcbQ ();

void initTidQ();
int isTidQFull();
int isTidQEmpty();
void pushTidQ(int item);
int popTidQ();


#endif /* INC_COMMON_H_ */

#include <common.h>

int totalEntries = (STACK_TOTAL_SIZE-MAIN_STACK_SIZE)/STACK_SIZE; //available blocks
char stackEntries[(STACK_TOTAL_SIZE-MAIN_STACK_SIZE)/STACK_SIZE]; //should all be set to 1 (available)
struct TcbQ tcbQ;
struct TidQ tidQ;
uint32_t* MSP_INIT_VAL; //stack start
TCB runningTask = {.state = DORMANT};

uint32_t* getTaskStartAddress(int index){ //index starting at 0 is mem after kernel!

	uint32_t ret = (uint32_t)MSP_INIT_VAL -(uint32_t)MAIN_STACK_SIZE - index*(uint32_t)STACK_SIZE;
	return ret;
}

void initTcbQ() {
  tcbQ.front = -1;
  tcbQ.rear = -1;
  tcbQ.size = MAX_TASKS;
}

int isTcbQEmpty() 
{ 
  return (tcbQ.front == -1 && tcbQ.rear == -1); 
}

int isTcbQFull() {
  return ((tcbQ.rear + 1) % tcbQ.size == tcbQ.front);
}

void pushTcbQ(TCB* tcb) {
  if (isTcbQFull()) {
    return;
  }

  if (isTcbQEmpty()) {
    tcbQ.front = 0;
    tcbQ.rear = 0;
  } else {
    tcbQ.rear = (tcbQ.rear + 1) % tcbQ.size;
  }

  tcbQ.queue[tcbQ.rear] = *tcb;
}

TCB popTcbQ() {
  if (isTcbQEmpty()) {
    TCB a;
    return a;
  }

  TCB tcb = tcbQ.queue[tcbQ.front];

  if (tcbQ.front == tcbQ.rear) {
    
    tcbQ.front = -1;
    tcbQ.rear = -1;
  } else {
    tcbQ.front = (tcbQ.front + 1) % tcbQ.size;
  }

  if(tcb.tid == 0 && !isTcbQEmpty()){ //null task pulled and other tasks left
	  pushTcbQ(&tcb);
	  tcb = popTcbQ();
  }

  return tcb;
}


void initTidQ() {
  tidQ.front = -1;
  tidQ.rear = -1;
  tidQ.size = MAX_TASKS;
}

int isTidQFull() {
  return ((tidQ.rear + 1) % tidQ.size == tidQ.front);
}

int isTidQEmpty() {
  return (tidQ.front == -1 && tidQ.rear == -1); 
}

void pushTidQ(int item) {

   if (isTidQFull()){
      return;
   }

  if (isTidQEmpty()) {

    tidQ.front = 0;
    tidQ.rear = 0;
  } 
  
  else {

    tidQ.rear = (tidQ.rear + 1) % tidQ.size;
  }

  tidQ.array[tidQ.rear] = item;
}

int popTidQ() {

  if (isTidQEmpty()){

      return -1; 
  }

  int item = tidQ.array[tidQ.front];

  if (tidQ.front == tidQ.rear) {

    tidQ.front = -1;
    tidQ.rear = -1;
  } 
  else {
    
    tidQ.front = (tidQ.front + 1) % tidQ.size;
  }

  return item;
}
#include <k_task.h>

int kernelInitalized = 0;
int kernelRunning = 0;
int taskExiting = 0;


void RunNullTask(){
	while(1){}
}

int osKernelStart(void){

	if(kernelRunning || !kernelInitalized) return RTX_ERR;

	//Pop and set Running Task
	TCB task1 = popTcbQ();

	task1.state = RUNNING;
	runningTask = task1;
	uint32_t* ptr = task1.stack_high;

	//Create context
	*(--ptr) = 1<<24;
	*(--ptr) = (uint32_t)task1.ptask;

	for(int i = 0; i<14; i++){
		*(--ptr) = 0xA;
	}

	//Push to CPU
	__asm("MSR PSP, %0" : : "r"(ptr));
	__asm("SVC #1");

	kernelRunning = 1;
	return RTX_OK;
}

void osKernelInit(void){

	initTcbQ();
	MSP_INIT_VAL = *(uint32_t**)0x0;

	for(int i=0 ; i<totalEntries; i++){
		stackEntries[i] = 1; //block is not being used (1)
	}

    initTidQ();
    for (int i = 0; i < MAX_TASKS; i++) {
        pushTidQ(i);
    }

	//create null task
	TCB nullTask;
	nullTask.ptask = RunNullTask;
	nullTask.state = READY;
	nullTask.stack_size = STACK_SIZE;

	osCreateTask(&nullTask);

	kernelInitalized = 1;
}


void startTask(void){

    uint32_t psp;
    __asm volatile("MOV %0, R0" : "=r" (psp)); //bottom of 16 regs in stack

    //Current Task take back to queue.
    if(!taskExiting){

		runningTask.state = READY;
		runningTask.stack_ptr = psp; //ask if address after or at psp
		pushTcbQ(&runningTask);
    }

    //Task dormat, unmark taken stack slots.
    else{

		//stack_size already multiple of 8!
		uint32_t numBlocks = runningTask.stack_size/STACK_SIZE;

    	SetStackArr(runningTask.startBlockIndex, numBlocks, 1);
		pushTidQ(runningTask.tid);

    	taskExiting = 0;
    }


    //Get New task
    TCB nextTask = popTcbQ();
    nextTask.state = RUNNING;
    runningTask = nextTask;

    //Never Ran before, needs context and set psp
    if (nextTask.stack_high == nextTask.stack_ptr)
    {
        uint32_t* ptr = nextTask.stack_high;

        *(--ptr) = 1<<24;
        *(--ptr) = (uint32_t)nextTask.ptask;

        for(int i = 0; i<14; i++){
            *(--ptr) = 0xA;
        }

        __asm volatile ("MSR PSP, %0\n" : : "r" (ptr));
    }

    else
    {
        uint32_t* ptr = nextTask.stack_ptr;
        __asm volatile ("MSR PSP, %0\n" : : "r" (ptr));
    }

    return;

}

void osYield (void)
{
    //Save Context, Calls StartTask() and Pushes to Cpu
    __asm("SVC #2");

}

int osTaskExit(void)
{
	if(runningTask.state == DORMANT){
		return RTX_ERR;
	}

	else{
		taskExiting = 1;
		osYield();
	}

}


int osCreateTask(TCB* task){

	if(task->stack_size < STACK_SIZE || isTcbQFull()){
		return RTX_ERR;
	}

	int wantedSpace = task->stack_size;

	//Find Contiguous Block, return start index
	int startIndex = CheckStackArr(wantedSpace);

	//Actual amount of Blocks given
	int spaceGiven = (wantedSpace + STACK_SIZE - 1) / STACK_SIZE;

	//Nothing Found
	if(startIndex < 0){
		return RTX_ERR;
	}

	int tid = popTidQ(&tidQ);

	//Set Block Found to all 0s, used.
	SetStackArr(startIndex, spaceGiven, 0);


	task->stack_high = getTaskStartAddress(startIndex);
	task->state = READY;
	task->tid = tid;
	task->stack_ptr = task->stack_high;
	task->stack_size = spaceGiven*(uint16_t)STACK_SIZE;
	task->startBlockIndex = startIndex;
	pushTcbQ(task);
	return RTX_OK;
}


//Find Contiguous Block of Stack Mem
int CheckStackArr(int size){

	int entriesNeeded = (size + STACK_SIZE - 1) / STACK_SIZE; //rounds up
	int start = 0;
	int entriesTemp = 0;

	for(int i=0 ; i<totalEntries; i++){

		if(stackEntries[i]){ //if its empty 

			entriesTemp++;
			if(entriesTemp == entriesNeeded) return start; //found blocks
		}

		else{ // resets counter
			entriesTemp = 0;
			start = i+1;
		}
	}

	return -1;
}

void SetStackArr(int start, int length, int val){

	for(int i=0; i<length; i++){
		stackEntries[i+start] = val;
	}
}


int osTaskInfo(task_t TID, TCB *task_copy) {

	if(runningTask.state == RUNNING && TID == runningTask.tid){

		*task_copy = runningTask;
		return RTX_OK;
	}

	if(isTcbQEmpty()){

		return RTX_ERR;
	}

    int i = tcbQ.front;

    while (1) {

        if (tcbQ.queue[i].tid == TID) {

        	*task_copy = tcbQ.queue[i];
            return RTX_OK;
        }

        if (i == tcbQ.rear) {
            break;
        }

        i = (i + 1) % tcbQ.size;
    }

	return RTX_ERR;
}

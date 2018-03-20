#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BEGINNING 0
#define END 1
#define READY 0
#define RUNNING 1 
#define BLOCKED 2
#define ARRIVING 3
#define CS_REMOVE 4
#define CS_BRING 5

typedef struct process { // struct for storing the data of a process
	char proc_id; // process id 
	int update_time; // time when state is completed
	int wait_start_time; // time when wait time has begun counting
	int turnaround_start_time; // time when turnaround time has begun counting
	int initial_arrive_time; // time when process arrices
	int cpu_burst_time; // time process wants to spend in CPU once
	int num_bursts; // number of times process wants to be in the CPU
	int io_time; // time process waits after being in CPU
	int state; // state process is in
} process;

void printState(int s) { // prints the state a process is in for debugging print function
	if (s == 0) printf("%-10s","READY");
	else if (s == 1) printf("%-10s","RUNNING");
	else if (s == 2) printf("%-10s","BLOCKED");
	else if (s == 3) printf("%-10s","ARRIVING");
	else if (s == 4) printf("%-10s","CS_REMOVE");
	else printf("%-10s","CS_BRING");
	fflush(stdout);
}

void debugPrintProcesses(process** processes, int n) { // prints parsed processes for debugging
	int i;
	for (i = 0; i < n; i++) {
		printf("%c|%-5d|",(*processes)[i].proc_id,(*processes)[i].initial_arrive_time);
   	fflush(stdout);
		printf("%-5d|%-5d|%-5d\n",(*processes)[i].cpu_burst_time,(*processes)[i].num_bursts,(*processes)[i].io_time);
   	fflush(stdout);
	}
	printf("\n");
 	fflush(stdout);	
}

void debugPrintQueue(process*** queue, int n) { // prints queue for debugging
	int i;
	for (i = 0; i < n; i++) {
		if ((*queue)[i] != NULL) {
			printf("%c|%-5d|",(*queue)[i]->proc_id,(*queue)[i]->initial_arrive_time);
			fflush(stdout);
			printf("%-5d|%-5d|%-5d  State: ",(*queue)[i]->cpu_burst_time,(*queue)[i]->num_bursts,(*queue)[i]->io_time);
			fflush(stdout);
	   	printState((*queue)[i]->state);
			printf("  Update at time %dms\n",(*queue)[i]->update_time);
			fflush(stdout);
		}
	}
}

int comparator(const void* a, const void* b) { // comparator to handle ties
	process* p = *((process**) a);
	process* q = *((process**) b);
	int diff = p->update_time - q->update_time;
	if (diff == 0) {
		diff = p->cpu_burst_time - q->cpu_burst_time;
		if (diff == 0) {
			diff = p->io_time - q->io_time;
			if (diff == 0) {
				diff = p->initial_arrive_time - q->initial_arrive_time;
				if (diff == 0) {
					return p->proc_id - q->proc_id;
				}
			}
		}
	}
	return diff;
}

void error() { // outputs error
	perror("ERROR");
	exit(EXIT_FAILURE);
}

process** createQueue(int n) {
	process** queue = (process**)calloc(n,sizeof(process*));
	int i;
	for (i = 0; i < n; i++) queue[i] = NULL;
	return queue;
}

void freeQueue(process*** queue, int n) {
	if ((*queue) == NULL) return;
	int i;
	for (i = 0; i < n; i++) if ((*queue)[i] != NULL) free((*queue)[i]);
	free(*queue);
}

void printQueue(process*** queue, int n) {
	if (n == 0) printf("[Q <empty>]\n");
	else {
		printf("[Q");
		fflush(stdout);
		int i;
		for (i = 0; i < n; i++) {
			if ((*queue)[i] != NULL) {
				printf(" %c",(*queue)[i]->proc_id);
				fflush(stdout);
			}
		}
		printf("]\n");
	}
	fflush(stdout);
}

void moveProcess(process** destination, process** source, int free_flag) { // moves process from source to destination, with the option to free source
	*destination = (process*)calloc(1,sizeof(process));
	memcpy(*destination,*source,sizeof(process));
	if (free_flag) {
		free(*source);
		*source = NULL;
	}
}

int addProcess(process*** queue, process p, int queue_capacity) { // adds process to queue
	int i;
	for (i = 0; i < queue_capacity; i++) {
		if ((*queue)[i] == NULL) {
			process* temp = &p;
			moveProcess(&(*queue)[i],&temp,0);
			temp = NULL;
			return i;
		}
	}
	return -1;
}

void updateQueue(process*** queue, int queue_capacity) { // moves elements in queue to the left
	if (queue_capacity == 1) return;
	int i,j;
	for (i = 1; i < queue_capacity; i++) {
		for (j = 0; j < i; j++) {
			if ((*queue)[j] == NULL && (*queue)[i] != NULL) {
				moveProcess(&(*queue)[j],&(*queue)[i],1);
				break;
			}
		}
	}
}

process* fileParser(int* n, const char** arg1) { // parses file into process struct
	FILE* inputFile = NULL;
	inputFile = fopen(*arg1,"r");
	if (inputFile == NULL) error();
	process* processes = (process*)calloc(26,sizeof(process));
	char buffer[100];
	memset(buffer,'\0',100);
	while (fgets(buffer,100,inputFile) != NULL) { // reads line from file, buffering 100 characters
		if (buffer[0] != '#' && buffer[0] != ' ' && buffer[0] != '\n' && buffer[1] == '|') { // checks to see if line can be parsed
			processes[*n].proc_id = buffer[0];
			char buffer2[100];
			memset(buffer2,'\0',100);
			int i = 2, j;
			for (j = 0; j < 100; i++, j++) { // parses for initial arrival time
				if (buffer[i] != '|') buffer2[j] = buffer[i];
				else {
					i++;
					processes[*n].initial_arrive_time = atoi(buffer2);
					processes[*n].update_time = 0;
					processes[*n].wait_start_time = 0;
					processes[*n].turnaround_start_time = 0;
					processes[*n].state = ARRIVING;
					break;
				} 
			}
			memset(buffer2,'\0',100);
			for (j = 0; j < 100; i++, j++) { // paraes for cpu-burst-time
				if (buffer[i] != '|') buffer2[j] = buffer[i];
				else {
					i++;
					processes[*n].cpu_burst_time = atoi(buffer2);
					break;
				} 
			}
			memset(buffer2,'\0',100);
			for (j = 0; j < 100; i++, j++) { // parses for num-bursts
				if (buffer[i] != '|') buffer2[j] = buffer[i];
				else {
					i++;
					processes[*n].num_bursts = atoi(buffer2);
					break;
				} 
			}
			memset(buffer2,'\0',100);
			for (j = 0; j < 100; i++, j++) { // parses for io-time
				if (buffer[i] != '\0') buffer2[j] = buffer[i];
				else {
					i++;
					processes[*n].io_time = atoi(buffer2);
					break;
				} 
			}
			memset(buffer,'\0',100);
			(*n)++;
		}
	}
	processes = (process*)realloc(processes, (*n) * (sizeof(process)));
	#ifdef DEBUG_MODE
		printf("\nprocesses parsed: %d\n",*n);
		debugPrintProcesses(&processes,*n);
	#endif
	fclose(inputFile);
	return processes;
}

void FCFS(process** processes, int n, int t_cs, float* sum_wait_time, float* sum_turnaround_time, int* context_swtiches) { // First Come First Serve Algorithm
	int i, real_t = 0, change = 0, terminated = 0, context_switching = 0, ready_capacity = 0, wait_capacity = 0;
	process** ready_queue = createQueue(n);
	process** wait_array = createQueue(n);
	process* CPU = NULL;
   printf("time %dms: Simulator started for FCFS ",real_t);
   fflush(stdout);
	printQueue(&ready_queue,ready_capacity);
	while (terminated < n) {
		for (i = 0; i < n - terminated; i++) { // loop for processes leaving the CPU and getting blocked
			if (CPU != NULL && CPU->update_time == real_t && CPU->state == RUNNING) { // process finished with CPU burst
				if (CPU->num_bursts == 0) {
					printf("time %dms: Process %c terminated ",real_t,CPU->proc_id);
					fflush(stdout);
				} else {
					if (CPU->num_bursts == 1) printf("time %dms: Process %c completed a CPU burst; %d burst to go ",real_t,CPU->proc_id,CPU->num_bursts);
					else printf("time %dms: Process %c completed a CPU burst; %d bursts to go ",real_t,CPU->proc_id,CPU->num_bursts);
					fflush(stdout);
					printQueue(&ready_queue,ready_capacity);
					printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ",real_t,CPU->proc_id,real_t + (t_cs / 2) + CPU->io_time);
					fflush(stdout);
				}
				printQueue(&ready_queue,ready_capacity);
				CPU->update_time += (t_cs / 2);
				CPU->state = CS_REMOVE;
				context_switching = 1;
				change = 1;
			}
			if (CPU != NULL && CPU->update_time == real_t && CPU->state == CS_REMOVE) { // process finished being context switched out of CPU, to be either termianted or blocked
				if (CPU->num_bursts == 0) terminated++;
				else {
					int pos = addProcess(&wait_array,*CPU,n);
					wait_array[pos]->update_time += wait_array[pos]->io_time;
					wait_array[pos]->state = BLOCKED;
					wait_capacity++;
				}
				*sum_turnaround_time += real_t - CPU->turnaround_start_time;
				#ifdef DBEUG_MODE
					printf("\nturnaround time for %c: %dms\n\n", CPU->proc_id, real_t - CPU->turnaround_start_time);
					fflush(stdout);
				#endif
				context_switching = 0;
				free(CPU);
				CPU = NULL;
				change = 1;
			}
			if (wait_array[i] != NULL && wait_array[i]->update_time == real_t && wait_array[i]->state == BLOCKED) { // process is finished with I/O and placed on ready queue
				int pos = addProcess(&ready_queue,*(wait_array[i]),n);
				free(wait_array[i]);
				wait_array[i] = NULL;
				ready_queue[pos]->turnaround_start_time = real_t;
				ready_queue[pos]->wait_start_time = real_t;
				ready_queue[pos]->update_time = real_t;
				ready_queue[pos]->state = READY;
				ready_capacity++;
				wait_capacity--;
				printf("time %dms: Process %c completed I/O; added to ready queue ",real_t,ready_queue[pos]->proc_id);
				fflush(stdout);
				printQueue(&ready_queue,ready_capacity);
				change = 1;
			}
			updateQueue(&ready_queue,n);
			updateQueue(&wait_array,n);
		}
		for (i = 0; i < n - terminated; i++) { // loop for processes arriving and entering the CPU
			if (ready_queue[i] != NULL && ready_queue[i]->update_time == real_t && ready_queue[i]->state == CS_BRING) { // proccess finished context switching into the CPU
				printf("time %dms: Process %c started using the CPU ",real_t,ready_queue[i]->proc_id);
				fflush(stdout);
				CPU = (process*)calloc(1,sizeof(process));
				memcpy(CPU,ready_queue[i],sizeof(process));
				free(ready_queue[i]);
				ready_queue[i] = NULL;
				ready_capacity--;
				updateQueue(&ready_queue,n);
				printQueue(&ready_queue,ready_capacity);
				CPU->update_time += CPU->cpu_burst_time;
				CPU->state = RUNNING;
				(CPU->num_bursts)--;
				context_switching = 0;
				change = 1;
			}
			if ((*processes)[i].initial_arrive_time == real_t && (*processes)[i].state == ARRIVING) { // process has arrived
				int pos = addProcess(&ready_queue,(*processes)[i],n);
				ready_queue[pos]->turnaround_start_time = real_t;
				ready_queue[pos]->wait_start_time = real_t;
				ready_queue[pos]->update_time = real_t;
				ready_queue[pos]->state = READY;
				ready_capacity++;
				printf("time %dms: Process %c arrived and added to ready queue ",real_t,ready_queue[pos]->proc_id);
				fflush(stdout);
				printQueue(&ready_queue,ready_capacity);
				change = 1;
			}
			if (CPU == NULL && context_switching == 0 && ready_queue[i] != NULL && ready_queue[i]->state == READY) { // process is able to use the CPU
				*sum_wait_time += real_t - ready_queue[i]->wait_start_time;
				#ifdef DEBUG_MODE
					printf("\nwait time for %c: %dms\n\n", ready_queue[i]->proc_id, real_t - ready_queue[i]->wait_start_time);
					fflush(stdout);
				#endif
				(*context_swtiches)++;
				context_switching = 1;
				ready_queue[i]->state = CS_BRING;
				ready_queue[i]->update_time = real_t + (t_cs / 2);
				change = 1;
			}
			updateQueue(&ready_queue,n);
		}
		if (change) { // if a process has been updated
			change = 0;
			qsort(ready_queue,ready_capacity,sizeof(process*),comparator); // clears ties
			#ifdef DEBUG_MODE
				if (terminated < n) { // debug prints of CPU and queues
					printf("\n--- Printing at time %dms\n",real_t);
					fflush(stdout);
					if (CPU != NULL) {
						printf("Printing CPU\n");
						fflush(stdout);
						process** temp = &CPU;
						debugPrintQueue(&temp,1);
						temp = NULL;
					} else {
						printf("Empty CPU\n");
						fflush(stdout);
					}
					if (ready_capacity > 0) {
						printf("Printing Ready Queue\n");
						fflush(stdout);
						debugPrintQueue(&ready_queue,ready_capacity);
					} else {
						printf("Empty Ready Queue\n");
						fflush(stdout);
					}
					if (wait_capacity > 0) {
						printf("Printing Wait Queue\n");
						fflush(stdout);
						debugPrintQueue(&wait_array,wait_capacity);
					} else {
						printf("Empty Wait Queue\n");
						fflush(stdout);
					}
					printf("--------------------------\n\n");
				   fflush(stdout);
				}
			#endif
		}
		if (terminated == n) break; // ends simulation when all processes have been terminated
		real_t++;
	}
	printf("time %dms: Simulator ended for FCFS\n",real_t);
	fflush(stdout);
	if (CPU != NULL) free(CPU);
	freeQueue(&ready_queue,n);
	freeQueue(&wait_array,n);
}




/*==================================================================================================*/

int comparator2(const void * a, const void* b) { // comparator to handle ties
	process* p = (process*) a;
	process* q = (process*) b;
	int diff = p->update_time - q->update_time;
	if (diff == 0) {
		diff = p->cpu_burst_time - q->cpu_burst_time;
		if (diff == 0) {
			diff = p->io_time - q->io_time;
			if (diff == 0) {
				diff = p->initial_arrive_time - q->initial_arrive_time;
				if (diff == 0) {
					return p->proc_id - q->proc_id;
				}
			}
		}
	}
	return diff;
}


void printQueue2(process* queue, int size){
	if (size == 0) printf("[Q <empty>]\n");
	else {
		printf("[Q");
		fflush(stdout);
		int i;
		for (i = 0; i < size; i++) {
			printf(" %c",queue[i].proc_id);
			fflush(stdout);
		}
		printf("]\n");
	}
	fflush(stdout);
}


process* add_to_queue(process* queue, int size, process* p){
	queue[size]=*p;
	#if D2
	printf("before adding size is %d",size);
	printQueue2(queue, size);
	#endif
	qsort(queue,size+1,sizeof(process),comparator2);
	#if D2
	printQueue2(queue, size);
	#endif
	return queue;
}

//return the processe being removed, not the queue!!!
process* remove_from_queue(process* queue, int size, int index){
	process* r=(process*)malloc(sizeof(process));
	*r=queue[index];
	for (int i = index; i < size-1; ++i)
	{
		queue[i]=queue[i+1];
	}
	return r;
}

void d_printq(process* queue, int size){
	for (int i = 0; i < size; ++i)
	{

		printf("%c|%-5d|",queue[i].proc_id,queue[i].initial_arrive_time);
		fflush(stdout);
		printf("%-5d|%-5d|%-5d  State: ",queue[i].cpu_burst_time,queue[i].num_bursts,queue[i].io_time);
		fflush(stdout);
	   	printState(queue[i].state);
		printf("  Update at time %dms\n",queue[i].update_time);
		fflush(stdout);
	}
}


/*
											    								 	        - --------------------------
-------- 	---------------------------------------       	---------------------------		|						  |
| CPU  |    |   cs_space[0]   |   cs_space[1]     |     	|    ready_queue           |  	|     IO_SPACE            |
--------    ---------------------------------------         --------------------------      |						  |	
			 switching out====>
			               <============switching in										---------------------------
*/

void SRT(process* processes, int n, int t_cs) { // Shortest Remaining Time Algorithm
	printf("\n\n\n");
	int finished=0;
	int t=0;
	int process_index=0;
	int ready_size=0;
	int io_size=0;
	int cs_status=0;
	int CPU_status=0;     //0: free   1:only out  2:only in  3: both
	int preempt_remaining_time=0;
	process* CPU=(process*)malloc(sizeof(process));
	process* ready_queue=(process*)malloc(26*sizeof(process));
	process* tmp=(process*)malloc(sizeof(process));
	process* io_space=(process*)malloc(26*sizeof(process));
	process* cs_space=(process*)malloc(2*sizeof(process));
	printf("time %dms: Simulator started for FCFS ",t);
	fflush(stdout);
	printQueue2(ready_queue,ready_size);
	while(finished!=n){
		for (int i = process_index; i < n; ++i)
		{
			//chekc if there is new arrival
			if (processes[i].initial_arrive_time==t){
				*tmp=processes[i];
				if (CPU_status==1 && ( (CPU->update_time-t)>tmp->cpu_burst_time )  && cs_status==0){
					cs_status=4;
					printf("time %dms: Process %c arrived and will preempt %c ",t,processes[i].proc_id, CPU->proc_id);
					fflush(stdout);
					printQueue2(ready_queue,ready_size);
					process_index++;
					preempt_remaining_time=CPU->update_time-t;
					cs_space[0]=*CPU;
					cs_space[1]=*tmp;
					cs_space[0].state=11;
					cs_space[0].wait_start_time=preempt_remaining_time;
					cs_space[0].update_time=t+4;
					cs_space[1].update_time=t+8;	
				}
				else{
					tmp->update_time=processes[i].cpu_burst_time;
					add_to_queue(ready_queue,ready_size,tmp);
					ready_size++;
					printf("time %dms: Process %c arrived and added to ready queue ",t,processes[i].proc_id);
					//qsort(ready_queue,ready_size,sizeof(process*),comparator2);
					process_index++;
					fflush(stdout);
					printQueue2(ready_queue,ready_size);	
					#if D2
					d_printq(ready_queue,ready_size);
					#endif					
				}
			}
		}
		//check all io space if someone is finishing io
		//add preepetion later
		for (int i = 0; i < io_size; ++i)
		{
			*tmp=io_space[i];
			if (io_space[i].update_time==t){
				if (CPU_status==1 && ( (CPU->update_time-t)>tmp->cpu_burst_time )  && cs_status==0){
					cs_status=4;
					printf("time %dms: Process %c completed I/O and will preempt %c ",t,io_space[i].proc_id, CPU->proc_id);
					fflush(stdout);
					printQueue2(ready_queue,ready_size);	
					process_index++;
					preempt_remaining_time=CPU->update_time-t;
					cs_space[0]=*CPU;
					cs_space[1]=*tmp;					
					cs_space[0].update_time=t+4;
					cs_space[1].update_time=t+8;	
					cs_space[0].state=11;
					cs_space[0].wait_start_time=preempt_remaining_time;
				}
				else{
					process* r=remove_from_queue(io_space,io_size,i);
					io_size--;
					r->update_time=r->cpu_burst_time;
					add_to_queue(ready_queue,ready_size,r);
					ready_size++;
					printf("time %dms: Process %c completed I/O; added to ready queue ",t,r->proc_id);
					printQueue2(ready_queue,ready_size);
				}
			}
		}
		//finishing up context switch
		//for only out case 
		if (cs_status==1 && cs_space[0].update_time==t){
			*tmp=cs_space[0];
			CPU_status=0;
			cs_status=0;
			if (cs_space[0].num_bursts==0){			
				finished++;
			}
			else{
				//add to IO
				tmp->update_time=t+tmp->io_time;
				add_to_queue(io_space,io_size,tmp);
				io_size++;
			}
		}
		//just finished cs for only in case
		if (cs_status==2 && cs_space[1].update_time==t){
			/*
			*CPU=cs_space[1];
			CPU->update_time=t+CPU->cpu_burst_time;
			cs_status=0;
			CPU_status=1;
			printf("time %dms: Process %c started using the CPU ",t,CPU->proc_id);
			printQueue2(ready_queue,ready_size);
			*/
			cs_status=0;
			*CPU=cs_space[1];
			CPU_status=1;
			if (cs_space[1].state==11){
				CPU->update_time=CPU->wait_start_time+t;
				CPU->state=0;
				printf("time %dms: Process %c started using the CPU with %dms remaining  ",t,CPU->proc_id,CPU->wait_start_time);
				fflush(stdout);
			}
			else{
				CPU->update_time=CPU->cpu_burst_time+t;
				printf("time %dms: Process %c started using the CPU ",t,CPU->proc_id);
				fflush(stdout);
			}
			printQueue2(ready_queue,ready_size);
		}
		//just finished cs for both in and out case
		if (cs_status==3){
			if (cs_space[0].update_time==t){
				*tmp=cs_space[0];
				if (cs_space[0].num_bursts==0){			
					finished++;
				}
				else{
					tmp->update_time=t+tmp->io_time;
					add_to_queue(io_space,io_size,tmp);
					io_size++;
				}
			}
			if (cs_space[1].update_time==t){
				cs_status=0;
				*CPU=cs_space[1];
				CPU_status=1;
				if (cs_space[1].state==11){
					CPU->update_time=CPU->wait_start_time+t;
					CPU->state=0;
					printf("time %dms: Process %c started using the CPU with %dms remaining  ",t,CPU->proc_id,CPU->wait_start_time);
					fflush(stdout);
				}
				else{
					CPU->update_time=CPU->cpu_burst_time+t;
					printf("time %dms: Process %c started using the CPU ",t,CPU->proc_id);
					fflush(stdout);
				}
				printQueue2(ready_queue,ready_size);
			}
		}
		if (cs_status==4){
			//printf("preemept mode\n");
			if (cs_space[0].update_time==t){
				*tmp=cs_space[0];
				tmp->update_time=tmp->wait_start_time;
				add_to_queue(ready_queue,ready_size,tmp);
				ready_size++;
			}
			if (cs_space[1].update_time==t){
				cs_status=0;
				*CPU=cs_space[1];
				CPU_status=1;
				if (cs_space[1].state==11){
					CPU->update_time=CPU->wait_start_time+t;
					CPU->state=0;
					printf("time %dms: Process %c started using the CPU with %dms remaining  ",t,CPU->proc_id,CPU->wait_start_time);
					fflush(stdout);
				}
				else{
					CPU->update_time=CPU->cpu_burst_time+t;
					printf("time %dms: Process %c started using the CPU ",t,CPU->proc_id);
					fflush(stdout);
				}
				printQueue2(ready_queue,ready_size);
			}
		}
		//just finished cs for other cases
		/*
		if ( (cs_status==3 || cs_status==1) && (cs_space[0].update_time==t) ){
			//finish the switching out part
			*tmp=cs_space[0];
			if (cs_space[0].num_bursts==0){
				printf("time %dms: Process %c terminated ",t,CPU->proc_id);
				printQueue2(ready_queue,ready_size);				
				finished++;
			}
			else{
				//add to IO
				tmp->update_time=t+tmp->io_time;
				add_to_queue(io_space,io_size,tmp);
				io_size++;
			}
			//you need to swith in, right?
			if (cs_status==3){
				*CPU=cs_space[1];
				CPU->update_time=CPU->cpu_burst_time+t;
				printf("time %dms: Process %c started using the CPU ",t,CPU->proc_id);
				fflush(stdout);
				printQueue2(ready_queue,ready_size);
				CPU_status=1;
			}
			else{
				CPU_status=0;
			}
			cs_status=0;
		}
*/
		//putting into context switch
		//CPu needs to be removed
		if (CPU_status==1 && CPU->update_time==t){
			//only out and no in
			cs_space[0]=*CPU;
			cs_space[0].update_time=t+4;
			CPU_status=0;
			cs_status=1;
			cs_space[0].num_bursts--;
			if (cs_space[0].num_bursts>0){
				printf("time %dms: Process %c completed a CPU burst; %d bursts to go ",t,cs_space[0].proc_id, cs_space[0].num_bursts);
				printQueue2(ready_queue,ready_size);
				printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ",t,cs_space[0].proc_id,t + t_cs/2  + CPU->io_time);
				printQueue2(ready_queue,ready_size);				
				//in and out
				if (ready_size!=0){
					cs_space[1]=ready_queue[0];		
					remove_from_queue(ready_queue,ready_size,0);
					#if D3
					d_printq(cs_space,2);
					#endif 
					ready_size--;
					cs_status=3;
					cs_space[1].update_time=t+8;
				}
				else{			
				}

			}
			if (cs_space[0].num_bursts==0){
				printf("time %dms: Process %c terminated ",t,CPU->proc_id);
				printQueue2(ready_queue,ready_size);	
			}
			/*
			if (cs_space[0].num_bursts>0){
				//wokr                 asdf asdf asdf asd f
				printf("time %dms: Process %c completed a CPU burst; %d bursts to go ",t,cs_space[0].proc_id, cs_space[0].num_bursts);
				printQueue2(ready_queue,ready_size);
				if (cs_status==3){
					printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ",t,cs_space[0].proc_id,t + t_cs  + CPU->io_time);
					fflush(stdout);
				}
				else{
					printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ",t,cs_space[0].proc_id,t + (t_cs / 2) + CPU->io_time);
					fflush(stdout);
				}
				printQueue2(ready_queue,ready_size);
			}
			*/
		}
		//only in
		if( (cs_status==0) && (CPU_status==0) && (ready_size>0)  ){
			cs_status=2;
			cs_space[1]=ready_queue[0];
			remove_from_queue(ready_queue,ready_size,0);
			ready_size--;
			cs_space[1].update_time=t+4;
			//printf("moving in, update_time=%d\n",cs_space[1].update_time);
		}
		if (finished!=n){
			t++;
		}
	}
	printf("time %dms: Simulator ended for SRT\n",t);
	free(tmp);
}

void RR(int t_slice, int rr_add) { // Round Robin Algorithm
	
}

void fileOutput(process** processes, int n, float** sum_wait_time, float** sum_turnaround_time, int** context_switches, int** preemptions, char const** path) {
	FILE* wFile = fopen(*path,"w");
	if (wFile == NULL) {
		free(*processes);
		free(*sum_wait_time);
		free(*sum_turnaround_time);
		free(*context_switches);
		free(*preemptions);
		error();
	}
	int i, cpu_burst_sum = 0, bursts = 0;
	for (i = 0; i < n; i++) {
		cpu_burst_sum += (*processes)[i].cpu_burst_time * (*processes)[i].num_bursts;
		bursts += (*processes)[i].num_bursts;
	}
	#ifdef DEBUG_MODE
   	printf("sum wait time: %f\nsum turnaround time: %f\nnumber of bursts: %d\n",(*sum_wait_time)[0],(*sum_turnaround_time)[0],bursts);
		fflush(stdout);
	#endif
	for (i = 0; i < 3; i++) {
		if (i == 0) fprintf(wFile,"Algorithm FCFS\n");
		else if (i == 1) fprintf(wFile,"Algorithm SRT\n");
		else fprintf(wFile,"Algorithm RR\n");
		fprintf(wFile,"-- average CPU burst time: %.2f ms\n",(float) cpu_burst_sum / bursts);
		fprintf(wFile,"-- average wait time: %.2f ms\n",(*sum_wait_time)[i] / bursts);
		fprintf(wFile,"-- average turnaround time: %.2f ms\n",(*sum_turnaround_time)[i] / bursts);
		fprintf(wFile,"-- total number of context switches: %d\n",(*context_switches)[i]);
		fprintf(wFile,"-- total number of preemptions: %d\n",(*preemptions)[i]);
	}
	fclose(wFile);
}

int main(int argc, char const *argv[]) {
	if (argc < 3) { // error handling for command-line arguments
		fprintf(stderr, "ERROR: Invalid arugment(s)\nUSAGE: ./a.out <input-file> <stats-output-file> [<rr-add>]\n");
		exit(EXIT_FAILURE);
   }
   int n = 0; // the number of processes to simulate
   int t_cs = 8; // context switch time (in milliseconds)
   int t_slice = 80; // time slice for RR algorithm (in milliseconds)
   int rr_add = END;  // determines whether process are added to beginning or end of RR ready queue
   if (argc > 3 && strcmp(argv[3],"BEGINNING") == 0) rr_add = BEGINNING;
   #ifdef DEBUG_MODE
   	if (rr_add == END) printf("rr_add is END\n");
   	else printf("rr_add is BEGINNING\n");
   	fflush(stdout);
   #endif
   process* processes = fileParser(&n, &argv[1]);
   float* sum_wait_time = (float*)calloc(n,sizeof(float));
   float* sum_turnaround_time = (float*)calloc(n,sizeof(float));
   int* context_switches = (int*)calloc(n,sizeof(int));
   int* preemptions = (int*)calloc(n,sizeof(int));
   preemptions[0] = 0; // FCFS is a non-preemptive algorithm


   FCFS(&processes,n,t_cs,&sum_wait_time[0],&sum_turnaround_time[0],&context_switches[0]);
   n=0;
   process* processes2 = fileParser(&n, &argv[1]);
   //d_printq(processes2,n);
   SRT(processes2,n, t_cs);


   RR(t_slice, rr_add);
   fileOutput(&processes,n,&sum_wait_time,&sum_turnaround_time,&context_switches,&preemptions,&argv[2]);
   free(processes);
   free(processes2);
   free(sum_wait_time);
   free(sum_turnaround_time);
   free(context_switches);
   free(preemptions);
	return EXIT_SUCCESS;
}
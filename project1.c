// Kaylan Johnson <johnsk18>
// Xiao Jiang <jiangx5>
// Ruowen Qin <qinr>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

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
	int burst_remaining_time; // time left to complete CPU burst for SRT alg
	int cpu_burst_time_remaining; // time left to complete CPU burst for RR alg
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
	if (diff == 0) return p->proc_id - q->proc_id;
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
	if (inputFile == NULL) {
		fprintf(stderr, "ERROR: Invalid input file format\n");
		exit(EXIT_FAILURE);
	}
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
					processes[*n].cpu_burst_time_remaining = atoi(buffer2);
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
					printf("time %dms: Process %c completed a CPU burst; %d burst%s to go ",real_t,CPU->proc_id,CPU->num_bursts, CPU->num_bursts == 1 ? "" : "s");
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
				#ifdef DEBUG_MODE
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
			if (CPU != NULL && CPU->update_time == real_t && CPU->state == CS_BRING) { // proccess finished context switching into the CPU
				printf("time %dms: Process %c started using the CPU ",real_t,CPU->proc_id);
				fflush(stdout);
				printQueue(&ready_queue,ready_capacity);
				CPU->update_time += CPU->cpu_burst_time;
				CPU->state = RUNNING;
				(CPU->num_bursts)--;
				context_switching = 0;
				change = 1;
			}
			if (ready_queue[i] != NULL && ready_queue[i]->update_time - (t_cs / 2) + 1 == real_t && ready_queue[i]->state == CS_BRING) { // process moves out of the ready queue, context switching into the CPU
				CPU = (process*)calloc(1,sizeof(process));
				memcpy(CPU,ready_queue[i],sizeof(process));
				free(ready_queue[i]);
				ready_queue[i] = NULL;
				ready_capacity--;
				updateQueue(&ready_queue,n);
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
			if (CPU == NULL && context_switching == 0 && ready_queue[i] != NULL && ready_queue[i]->state == READY) { // process is able to use the CPU, beginning to context switch
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
				if (terminated < n) { // debug prints of CPU, ready queue, and wait array
					printf("\n--- Printing at time %dms\n%s CPU\n",real_t, CPU != NULL ? "Printing" : "Empty");
					fflush(stdout);
					if (CPU != NULL) {
						process** temp = &CPU;
						debugPrintQueue(&temp,1);
						temp = NULL;
					}
					printf("%s Ready Queue\n", ready_capacity > 0 ? "Printing" : "Empty");
					fflush(stdout);
					if (ready_capacity > 0)	debugPrintQueue(&ready_queue,ready_capacity);
					printf("%s Wait Array\n", wait_capacity > 0 ? "Printing" : "Empty");
					fflush(stdout);
					if (wait_capacity > 0) debugPrintQueue(&wait_array,wait_capacity);
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

int comparator2(const void* a, const void* b) { // comparator to handle ties
	process* p = (process*) a;
	process* q = (process*) b;
	int diff = p->update_time - q->update_time;
	if (diff == 0) return p->proc_id - q->proc_id;
	return diff;
}

void printQueue2(process* queue, int size) {
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

process* add_to_queue(process* queue, int size, process* p) {
	queue[size]=*p;
	p=NULL;
	qsort(queue,size+1,sizeof(process),comparator2);
	return queue;
}

process* remove_return_from_queue(process* queue, int size, int index) { // return the processe being removed, not the queue!!!
	process* r=(process*)malloc(sizeof(process));
	*r=queue[index];
	int i;
	for (i = index; i < size-1; ++i) queue[i]=queue[i+1];
	return r;
}

void remove_from_queue(process* queue, int size, int index) {
	int i;
	for (i = index; i < size-1; ++i) queue[i]=queue[i+1];
}

void d_printq(process* queue, int size) {
	int i;
	for (i = 0; i < size; ++i) {
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

void SRT(process* processes, int n, int t_cs, float * sum_wait_time, float* sum_turnaround_time, int* context_switches, int* preemptions) { // Shortest Remaining Time Algorithm
	int finished=0, t=0, process_index=0, ready_size=0, io_size=0, preempt_remaining_time=0, total_burst=0, cs_status=0, cs_n=0, preempts=0, CPU_status=0;     //0: free   1:only out  2:only in  3: both
	float total_turnaround=0.0, total_wait=0.0, tw=0.0;
	process* CPU=(process*)malloc(sizeof(process));
	process* ready_queue=(process*)malloc(26*sizeof(process));
	process* tmp=(process*)malloc(sizeof(process));
	process* io_space=(process*)malloc(26*sizeof(process));
	process* cs_space=(process*)malloc(2*sizeof(process));
	printf("\ntime %dms: Simulator started for SRT ",t);
	fflush(stdout);
	printQueue2(ready_queue,ready_size);
	while(finished!=n){
		int i;
		//putting into context switch. CPU needs to be removed
		tw+=ready_size;
		if (CPU_status==1 && CPU->update_time==t){
			//only out and no in
			cs_space[0]=*CPU;
			cs_space[0].update_time=t+4;
			CPU_status=0;
			cs_status=1;
			cs_space[0].num_bursts--;
			if (cs_space[0].num_bursts>0){
				if (cs_space[0].num_bursts==1) printf("time %dms: Process %c completed a CPU burst; %d burst to go ",t,cs_space[0].proc_id, cs_space[0].num_bursts);					
				else printf("time %dms: Process %c completed a CPU burst; %d bursts to go ",t,cs_space[0].proc_id, cs_space[0].num_bursts);
				printQueue2(ready_queue,ready_size);
				printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ",t,cs_space[0].proc_id,t + t_cs/2  + CPU->io_time);
				printQueue2(ready_queue,ready_size);				
				//in and out
				if (ready_size!=0){
					cs_space[1]=ready_queue[0];		
					total_wait+=t-ready_queue[0].wait_start_time;//add up the wait time
					remove_from_queue(ready_queue,ready_size,0);
					#if D3
					d_printq(cs_space,2);
					#endif 
					ready_size--;
					cs_status=3;
					cs_space[1].update_time=t+8;
					tw+=4;
				}
			}
			if (cs_space[0].num_bursts==0){
				printf("time %dms: Process %c terminated ",t,CPU->proc_id);
				printQueue2(ready_queue,ready_size);
				total_turnaround+=t;
				total_turnaround+=4; //last cs	
			}
		}
		//check all io space if someone is finishing io. add preepetion later
		for (i = 0; i < io_size; ++i){
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
					cs_space[0].burst_remaining_time=preempt_remaining_time;
					preempts++;
				} else{
					*tmp=io_space[i];
					remove_from_queue(io_space,io_size,i);
					io_size--;
					tmp->update_time=tmp->cpu_burst_time;
					tmp->wait_start_time=t;
					add_to_queue(ready_queue,ready_size,tmp);
					ready_size++;
					printf("time %dms: Process %c completed I/O; added to ready queue ",t,tmp->proc_id);
					printQueue2(ready_queue,ready_size);
				}
			}
		}
		for (i = process_index; i < n; ++i){
			//check if there is new arrival
			if (processes[i].initial_arrive_time==t){
				*tmp=processes[i];
				total_burst=total_burst+tmp->num_bursts; 
				total_turnaround-=t;
				total_turnaround-=(tmp->num_bursts-1)*(tmp->io_time);   
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
					cs_space[0].burst_remaining_time=preempt_remaining_time;
					cs_space[0].update_time=t+4;
					cs_space[1].update_time=t+8;	
					preempts++;
				} else{
					tmp->update_time=processes[i].cpu_burst_time;
					tmp->wait_start_time=t;
					add_to_queue(ready_queue,ready_size,tmp);
					ready_size++;
					printf("time %dms: Process %c arrived and added to ready queue ",t,processes[i].proc_id);
					process_index++;
					fflush(stdout);
					printQueue2(ready_queue,ready_size);	
				}
			}
		}
		//finishing up context switch for only out case 
		if (cs_status==1 && cs_space[0].update_time==t){
			*tmp=cs_space[0];
			CPU_status=0;
			cs_status=0;
			if (cs_space[0].num_bursts==0) finished++;
			else{
				//add to IO
				tmp->update_time=t+tmp->io_time;
				add_to_queue(io_space,io_size,tmp);
				io_size++;
			}
		}
		//just finished cs for only in case
		if (cs_status==2 && cs_space[1].update_time==t){
			cs_status=0;
			*CPU=cs_space[1];
			CPU_status=1;
			cs_n++;
			if (cs_space[1].state==11){
				CPU->update_time=CPU->burst_remaining_time+t;
				CPU->state=0;
				printf("time %dms: Process %c started using the CPU with %dms remaining ",t,CPU->proc_id,CPU->burst_remaining_time);
				fflush(stdout);
			} else{
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
				if (cs_space[0].num_bursts==0) finished++;
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
				cs_n++;
				if (cs_space[1].state==11){
					CPU->update_time=CPU->burst_remaining_time+t;
					CPU->state=0;
					printf("time %dms: Process %c started using the CPU with %dms remaining ",t,CPU->proc_id,CPU->burst_remaining_time);
					fflush(stdout);
				} else{
					CPU->update_time=CPU->cpu_burst_time+t;
					printf("time %dms: Process %c started using the CPU ",t,CPU->proc_id);
					fflush(stdout);
				}
				printQueue2(ready_queue,ready_size);
			}
		}
		if (cs_status==4){
			if (cs_space[0].update_time==t){
				*tmp=cs_space[0];
				tmp->update_time=tmp->burst_remaining_time;
				tmp->wait_start_time=t;
				add_to_queue(ready_queue,ready_size,tmp);
				ready_size++;
			}
			if (cs_space[1].update_time==t){
				cs_status=0;
				*CPU=cs_space[1];
				CPU_status=1;
				cs_n++;
				if (cs_space[1].state==11){
					CPU->update_time=CPU->burst_remaining_time;
					CPU->state=0;
					printf("time %dms: Process %c started using the CPU with %dms remaining  ",t,CPU->proc_id,CPU->burst_remaining_time);
					fflush(stdout);
				} else{
					CPU->update_time=CPU->cpu_burst_time+t;
					printf("time %dms: Process %c started using the CPU ",t,CPU->proc_id);
					fflush(stdout);
				}
				printQueue2(ready_queue,ready_size);
			}
		}
		//only in
		if( (cs_status==0) && (CPU_status==0) && (ready_size>0)  ){
			cs_status=2;
			cs_space[1]=ready_queue[0];
			total_wait+=t-ready_queue[0].wait_start_time;
			remove_from_queue(ready_queue,ready_size,0);
			ready_size--;
			cs_space[1].update_time=t+4;
		}
		if (finished!=n) t++;
	}
	printf("time %dms: Simulator ended for SRT\n",t);
	*sum_turnaround_time=total_turnaround;
	*sum_wait_time=tw;
	*context_switches=cs_n;
	*preemptions=preempts;
	free(CPU);
	free(ready_queue);
	free(io_space);
	free(cs_space);
	free(tmp);
}

/*==================================================================================================*/

void debugPrintQueueRR(process ***queue, int n) { // prints queue for debugging
	int i;
	for (i = 0; i < n; i++) {
		if ((*queue)[i] != NULL) {
			printf("%c|%-5d|", (*queue)[i]->proc_id, (*queue)[i]->initial_arrive_time);
			fflush(stdout);
			printf("%-5d|%-5d|%-5d|%-5d  State: ", (*queue)[i]->cpu_burst_time, (*queue)[i]->cpu_burst_time_remaining, (*queue)[i]->num_bursts, (*queue)[i]->io_time);
			fflush(stdout);
			printState((*queue)[i]->state);
			printf("  Update at time %dms\n", (*queue)[i]->update_time);
			fflush(stdout);
		}
	}
}

int addProcessToFirst(process ***queue, process p, int queue_capacity) {
	int i,j;
	process *temp = &p;
	for (i = 0; i < queue_capacity; ++i) {
		if ((*queue)[i] == NULL) {
			for (j = i; j > 0; --j) {
				moveProcess(&(*queue)[j], &(*queue)[j - 1], 1);
			}
			moveProcess(&(*queue)[0], &temp, 0);
			temp = NULL;
			return 0;
		}
	}
	return -1;
}

void RR(process **processes, int n, int t_cs, float *sum_wait_time, float *sum_turnaround_time, int *context_swtiches, int *preemptions, int t_slice, int rr_add) { // Round Robin Algorithm
	// rr_add define whether processes are added to the end or the beginning of the ready queue when they arrive
	int i, real_t = 0, terminated = 0, ready_capacity = 0, wait_capacity = 0;
	bool change = false, context_switching = false;
	process **ready_queue = createQueue(n);
	process **wait_array = createQueue(n);
	process *CPU = NULL; // current working process
	int slice_count = 0;
	printf("\ntime %dms: Simulator started for RR ", real_t);
	fflush(stdout);
	printQueue(&ready_queue, ready_capacity);
	while (terminated < n) {
		for (i = 0; i < n - terminated; ++i) {	// loop for processes leaving the CPU and getting blocked
			if (CPU != NULL && CPU->update_time == real_t && CPU->state == RUNNING) {
					bool IOCheck = false;
					bool terminatedCheck = false;
					if (CPU->cpu_burst_time_remaining == 0) {
						(CPU->num_bursts)--;
						if (CPU->num_bursts == 0) {
							terminatedCheck = true;
							printf("time %dms: Process %c terminated ", real_t, CPU->proc_id);
							printQueue(&ready_queue, ready_capacity);
							fflush(stdout);
						} else {
							IOCheck = true;
							printf("time %dms: Process %c completed a CPU burst; %d burst%s to go ", real_t, CPU->proc_id, CPU->num_bursts, CPU->num_bursts == 1 ? "" : "s");
							fflush(stdout);
							printQueue(&ready_queue, ready_capacity);
							printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms ", real_t, CPU->proc_id, real_t + (t_cs / 2) + CPU->io_time);
							printQueue(&ready_queue, ready_capacity);
							fflush(stdout);
						}
					}
					if (ready_capacity == 0 && CPU->num_bursts != 0 && !IOCheck) {
						// also not need to move to IO
						printf("time %dms: Time slice expired; no preemption because ready queue is empty ", real_t);
						fflush(stdout);
						printQueue(&ready_queue, ready_capacity);
						// no process in ready queue
						if (CPU->cpu_burst_time_remaining > t_slice) {
							CPU->update_time += t_slice;
							CPU->cpu_burst_time_remaining -= t_slice;
						} else {
							CPU->update_time += CPU->cpu_burst_time_remaining;
							CPU->cpu_burst_time_remaining = 0;
						}
					} else {
						if (!IOCheck && !terminatedCheck) {	// been preemptions by other process
							printf("time %dms: Time slice expired; process %c preempted with %dms to go ", real_t, CPU->proc_id, CPU->cpu_burst_time_remaining);
							fflush(stdout);
							printQueue(&ready_queue, ready_capacity);
							(*preemptions)++;
						}
						CPU->update_time += (t_cs / 2);
						CPU->state = CS_REMOVE;
						context_switching = true;
						change = true;
					}
			}
			if (CPU != NULL && CPU->update_time == real_t && CPU->state == CS_REMOVE) {
					if (CPU->cpu_burst_time_remaining == 0) { // run out
						*sum_turnaround_time += real_t - CPU->turnaround_start_time;
						#ifdef DEBUG_MODE
							printf("\nturnaround time for %c: %dms\n\n", CPU->proc_id, real_t - CPU->turnaround_start_time);
							fflush(stdout);
						#endif
						if (CPU->num_bursts == 0) terminated++;
						else {
							int pos = addProcess(&wait_array, *CPU, n);
							wait_array[pos]->cpu_burst_time_remaining = wait_array[pos]->cpu_burst_time;
							wait_array[pos]->update_time += wait_array[pos]->io_time;
							wait_array[pos]->wait_start_time = real_t;
							wait_array[pos]->state = BLOCKED;
							wait_capacity++;
						}
					} else { // add to ready queue
						int pos = addProcess(&ready_queue, *CPU, n);
						ready_queue[pos]->state = READY;
						ready_queue[pos]->wait_start_time = real_t;
						ready_capacity++;
					}
					context_switching = false;
					free(CPU);
					CPU = NULL;
					change = true;
				}
			if (wait_array[i] != NULL && wait_array[i]->update_time == real_t && wait_array[i]->state == BLOCKED) { // process is finished with I/O and placed on ready queue
				int pos = 0;
				if (rr_add) pos = addProcess(&ready_queue, *(wait_array[i]), n); // add to end of queue
				else pos = addProcessToFirst(&ready_queue, *(wait_array[i]), n);
				free(wait_array[i]);
				wait_array[i] = NULL;
				ready_queue[pos]->turnaround_start_time = real_t;
				ready_queue[pos]->wait_start_time = real_t;
				ready_queue[pos]->update_time = real_t;
				ready_queue[pos]->state = READY;
				ready_capacity++;
				wait_capacity--;
				printf("time %dms: Process %c completed I/O; added to ready queue ", real_t, ready_queue[pos]->proc_id);
				fflush(stdout);
				printQueue(&ready_queue, ready_capacity);
				change = true;
				updateQueue(&ready_queue,n);
				updateQueue(&wait_array,n);
			}
		}
		for (i = 0; i < n - terminated; ++i) { // loop for processes arriving and entering the CPU
			if (context_switching && CPU != NULL && CPU->update_time == real_t && CPU->state == CS_BRING) {
				// proccess finished context switching into the CPU
				if (CPU->cpu_burst_time_remaining == CPU->cpu_burst_time) printf("time %dms: Process %c started using the CPU ", real_t, CPU->proc_id);
				else printf("time %dms: Process %c started using the CPU with %dms remaining ", real_t, CPU->proc_id, CPU->cpu_burst_time_remaining);
				fflush(stdout);
				printQueue(&ready_queue, ready_capacity);
				if (CPU->cpu_burst_time_remaining > t_slice) {
					CPU->update_time += t_slice;
					CPU->cpu_burst_time_remaining -= t_slice;
				} else {
					CPU->update_time += CPU->cpu_burst_time_remaining;
					CPU->cpu_burst_time_remaining = 0;
				}
				CPU->state = RUNNING;
				context_switching = false;
				change = true;
			}
			if (ready_queue[i] != NULL && ready_queue[i]->update_time - (t_cs / 2) + 1 == real_t && ready_queue[i]->state == CS_BRING) { // process moves out of the ready queue, context switching into the CPU
				CPU = (process *) calloc(1, sizeof(process));
				memcpy(CPU, ready_queue[i], sizeof(process));
				free(ready_queue[i]);
				ready_queue[i] = NULL;
				ready_capacity--;
				updateQueue(&ready_queue, n);
				change = true;
			}
			if ((*processes)[i].initial_arrive_time == real_t && (*processes)[i].state == ARRIVING) { // process has arrived
				int pos = 0;
				if (rr_add) pos = addProcess(&ready_queue, (*processes)[i], n); // add to end of queue
				else pos = addProcessToFirst(&ready_queue, (*processes)[i], n);
				ready_queue[pos]->turnaround_start_time = real_t;
				ready_queue[pos]->wait_start_time = real_t;
				ready_queue[pos]->update_time = real_t;
				ready_queue[pos]->state = READY;
				ready_capacity++;
				printf("time %dms: Process %c arrived and added to ready queue ", real_t, ready_queue[pos]->proc_id);
				fflush(stdout);
				printQueue(&ready_queue, ready_capacity);
				change = true;
				updateQueue(&ready_queue, n);
			}
			if (rr_add && !context_switching && CPU == NULL && ready_queue[i] != NULL && ready_queue[i]->state == READY) { // process is able to use the CPU, beginning to context switch, when rr_add is END
				*sum_wait_time += real_t - ready_queue[i]->wait_start_time;
				#ifdef DEBUG_MODE
					printf("\nwait time for %c: %dms\n\n", ready_queue[i]->proc_id, real_t - ready_queue[i]->wait_start_time);
					fflush(stdout);
				#endif
				(*context_swtiches)++;
				context_switching = true;
				ready_queue[i]->state = CS_BRING;
				ready_queue[i]->update_time = real_t + (t_cs / 2);
				change = true;
			}
			updateQueue(&ready_queue, n); // fill blank place
		}
		if (!rr_add && !context_switching && CPU == NULL && ready_queue[0] != NULL && ready_queue[0]->state == READY) { // process is able to use the CPU, beginning to context switch, when rr_add is BEGINNING
			*sum_wait_time += real_t - ready_queue[0]->wait_start_time;
			#ifdef DEBUG_MODE
				printf("\nwait time for %c: %dms\n\n", ready_queue[0]->proc_id, real_t - ready_queue[0]->wait_start_time);
				fflush(stdout);
			#endif
			(*context_swtiches)++;
			context_switching = true;
			ready_queue[0]->state = CS_BRING;
			ready_queue[0]->update_time = real_t + (t_cs / 2);
			change = true;
			updateQueue(&ready_queue, n); // fill blank place
		}
		if (change) { // if a process has been updated
			change = false;
			#ifdef DEBUG_MODE
				if (terminated < n) { // debug prints of CPU, ready queue, and wait array
					printf("\n--- Printing at time %dms\n%s CPU\n", real_t, CPU != NULL ? "Printing" : "Empty");
					fflush(stdout);
					if (CPU != NULL) {
						process **temp = &CPU;
						debugPrintQueueRR(&temp, 1);
						temp = NULL;
					}
					printf("%s Ready Queue\n", ready_capacity > 0 ? "Printing" : "Empty");
					fflush(stdout);
					if (ready_capacity > 0) debugPrintQueueRR(&ready_queue, ready_capacity);
					printf("%s Wait Array\n", wait_capacity > 0 ? "Printing" : "Empty");
					fflush(stdout);
					if (wait_capacity > 0) debugPrintQueueRR(&wait_array, wait_capacity);
					printf("--------------------------\n\n");
					fflush(stdout);
				}
			#endif
		}
		if (terminated == n) break;
		real_t++;
		slice_count++;
	}
	printf("time %dms: Simulator ended for RR\n", real_t);
	fflush(stdout);
	if (CPU != NULL) free(CPU);
	freeQueue(&ready_queue, n);
	freeQueue(&wait_array, n);
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
		fprintf(wFile,"Algorithm %s\n", i == 0 ? "FCFS" : i == 1 ? "SRT" : "RR");
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
	int rr_add = (argc > 3 && strcmp(argv[3],"BEGINNING") == 0) ? BEGINNING : END;  // determines whether process are added to beginning or end of RR ready queue
	#ifdef DEBUG_MODE
		printf("rr_add is %s\n", rr_add == END ? "END" : "BEGINNING");
		fflush(stdout);
	#endif
	process* processes = fileParser(&n, &argv[1]);
	float* sum_wait_time = (float*)calloc(3,sizeof(float));
	float* sum_turnaround_time = (float*)calloc(3,sizeof(float));
	int* context_switches = (int*)calloc(3,sizeof(int));
	int* preemptions = (int*)calloc(3,sizeof(int));
	preemptions[0] = 0; // FCFS is a non-preemptive algorithm
	FCFS(&processes,n,t_cs,&sum_wait_time[0],&sum_turnaround_time[0],&context_switches[0]);
	SRT(processes,n,t_cs,&sum_wait_time[1],&sum_turnaround_time[1],&context_switches[1],&preemptions[1]);
	RR(&processes, n, t_cs, &sum_wait_time[2], &sum_turnaround_time[2], &context_switches[2], &preemptions[2], t_slice, rr_add);
	fileOutput(&processes,n,&sum_wait_time,&sum_turnaround_time,&context_switches,&preemptions,&argv[2]);
	free(processes);
	free(sum_wait_time);
	free(sum_turnaround_time);
	free(context_switches);
	free(preemptions);
	return EXIT_SUCCESS;
}
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
	char proc_id;
	int update_time;
	int initial_arrive_time;
	int cpu_burst_time;
	int num_bursts;
	int io_time;
	int state;
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
	exit(1);
}

void createQueue(process*** queue, int n) {
	*queue = (process**)calloc(n,sizeof(process*));
	int i;
	for (i = 0; i < n; i++) (*queue)[i] = NULL;
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

void fileParser(process** processes, int* n, const char** arg1) { // parses file into process struct
	FILE* inputFile = NULL;
	inputFile = fopen(*arg1,"r");
	if (inputFile == NULL) error();
	*processes = (process*)calloc(26,sizeof(process));
	char buffer[100];
	memset(buffer,'\0',100);
	while (fgets(buffer,100,inputFile) != NULL) { // reads line from file, buffering 100 characters
		if (buffer[0] != '#' && buffer[0] != ' ' && buffer[0] != '\n' && buffer[1] == '|') { // checks to see if line can be parsed
			(*processes)[*n].proc_id = buffer[0];
			char buffer2[100];
			memset(buffer2,'\0',100);
			int i = 2, j;
			for (j = 0; j < 100; i++, j++) { // parses for initial arrival time
				if (buffer[i] != '|') buffer2[j] = buffer[i];
				else {
					i++;
					(*processes)[*n].initial_arrive_time = atoi(buffer2);
					(*processes)[*n].update_time = 0;
					(*processes)[*n].state = ARRIVING;
					break;
				} 
			}
			memset(buffer2,'\0',100);
			for (j = 0; j < 100; i++, j++) { // paraes for cpu-burst-time
				if (buffer[i] != '|') buffer2[j] = buffer[i];
				else {
					i++;
					(*processes)[*n].cpu_burst_time = atoi(buffer2);
					break;
				} 
			}
			memset(buffer2,'\0',100);
			for (j = 0; j < 100; i++, j++) { // parses for num-bursts
				if (buffer[i] != '|') buffer2[j] = buffer[i];
				else {
					i++;
					(*processes)[*n].num_bursts = atoi(buffer2);
					break;
				} 
			}
			memset(buffer2,'\0',100);
			for (j = 0; j < 100; i++, j++) { // parses for io-time
				if (buffer[i] != '\0') buffer2[j] = buffer[i];
				else {
					i++;
					(*processes)[*n].io_time = atoi(buffer2);
					break;
				} 
			}
			memset(buffer,'\0',100);
			(*n)++;
		}
	}
	*processes = realloc(*processes, (*n) * (sizeof(process)));
	#ifdef DEBUG_MODE
		printf("\nprocceses = %d\n",*n);
		debugPrintProcesses(processes,*n);
	#endif
	fclose(inputFile);
}

void FCFS_alg(process** processes, int n, int t_cs, int* context_swtiches, int* preemptions) { // First Come First Server Alg
	int i, real_t = 0, change = 0, terminated = 0, context_switching = 0, ready_capacity = 0;
	process** ready_queue = NULL;
	process** wait_array = NULL;
	createQueue(&ready_queue,n);
	createQueue(&wait_array,n);
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
				(*context_swtiches)++;
				context_switching = 1;
				change = 1;
			}
			if (CPU != NULL && CPU->update_time == real_t && CPU->state == CS_REMOVE) { // process finished being context switched out of CPU, to be either termianted or blocked
				if (CPU->num_bursts == 0) terminated++;
				else {
					int pos = addProcess(&wait_array,*CPU,n);
					wait_array[pos]->update_time += wait_array[pos]->io_time;
					wait_array[pos]->state = BLOCKED;
				}
				context_switching = 0;
				free(CPU);
				CPU = NULL;
				change = 1;
			}
			if (wait_array[i] != NULL && wait_array[i]->update_time == real_t && wait_array[i]->state == BLOCKED) { // process is finished with I/O and placed on ready queue
				int pos = addProcess(&ready_queue,*(wait_array[i]),n);
				free(wait_array[i]);
				wait_array[i] = NULL;
				ready_queue[pos]->update_time = real_t;
				ready_queue[pos]->state = READY;
				ready_capacity++;
				printf("time %dms: Process %c completed I/O; added to ready queue ",real_t,ready_queue[pos]->proc_id);
				fflush(stdout);
				printQueue(&ready_queue,ready_capacity);
				change = 1;
			}
			updateQueue(&ready_queue,n);
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
				ready_queue[pos]->update_time = real_t;
				ready_queue[pos]->state = READY;
				ready_capacity++;
				printf("time %dms: Process %c arrived and added to ready queue ",real_t,ready_queue[pos]->proc_id);
				fflush(stdout);
				printQueue(&ready_queue,ready_capacity);
				change = 1;
			}
			if (CPU == NULL && context_switching == 0 && ready_queue[i] != NULL && ready_queue[i]->state == READY) { // process is able to use the CPU
				(*context_swtiches)++;
				context_switching = 1;
				ready_queue[i]->state = CS_BRING;
				ready_queue[i]->update_time = real_t + (t_cs / 2);
				change = 1;
			}
			updateQueue(&ready_queue,n);
		}
		if (change) {
			change = 0;
			qsort(ready_queue,ready_capacity,sizeof(process*),comparator); // clears ties
			#ifdef DEBUG_MODE
				if (terminated < n) {
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
					printf("--------------------------\n\n");
				   fflush(stdout);
				}
			#endif
		}
		if (terminated == n) break;
		real_t++;
	}
	printf("time %dms: Simulator ended for FCFS\n",real_t);
	fflush(stdout);
	freeQueue(&ready_queue,n);
	freeQueue(&wait_array,n);
}

void SRT() {
	
}

void RR(int rr_add) {
	
}

int main(int argc, char const *argv[]) {
	if (argc < 3) { // error handling for command-line arguments
		fprintf(stderr, "ERROR: Invalid arugment(s)\nUSAGE: ./a.out <input-file> <stats-output-file> [<rr-add>]\n");
		exit(EXIT_FAILURE);
   }
   
   int n = 0; // the number of processes to simulate
   int t_cs = 8; // context switch time (in milliseconds)
   // int t_slive = 80; // time slice for RR algorithm (in milliseconds)
   int rr_add = END;  // determines whether process are added to beginning or end of RR ready queue
   if (argc > 3 && strcmp(argv[3],"BEGINNING") == 0) rr_add = BEGINNING;
   #ifdef DEBUG_MODE
   	printf("rr_add is %d\n\n",rr_add);
   	fflush(stdout);
   #endif
   
   process* processes = NULL;
   fileParser(&processes, &n, &argv[1]);
   int* context_switches = (int*)calloc(n,sizeof(int));
   int* preemptions = (int*)calloc(n,sizeof(int));
   FCFS_alg(&processes,n,t_cs,&context_switches[0],&preemptions[0]);
   SRT();
   RR(rr_add);
   free(processes);
   free(context_switches);
   free(preemptions);
	return EXIT_SUCCESS;
}
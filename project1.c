#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BEGINNING 0
#define END 1
#define READY 0
#define RUNNING 1 
#define BLOCKED 2
#define TERMINATED 3
#define CONTEXT_SWITCH 4


typedef struct process { // struct for storing the data of a process
	char* proc_id;
	int current_time;
	int initial_arrive_time;
	int cpu_burst_time;
	int num_bursts;
	int io_time;
	int state;
} process;

void printProcesses(process** processes, int n) {
	int i;
	for (i = 0; i < n; i++) {
		printf("Process %d: %s|%d|",i+1,(*processes)[i].proc_id,(*processes)[i].current_time);
   	fflush(stdout);
		printf("%d|%d|",(*processes)[i].initial_arrive_time,(*processes)[i].cpu_burst_time);
   	fflush(stdout);
		printf("%d|%d --- State %d\n",(*processes)[i].num_bursts,(*processes)[i].io_time,(*processes)[i].state);
   	fflush(stdout);
	}
	printf("\n");
   fflush(stdout);
}

int comparator(const void* a, const void* b) { // comparator to handle ties
	process* p = (process*) a;
	process* q = (process*) b;
	if (p->state == TERMINATED) return 1;
	if (q->state == TERMINATED) return -1;
	int diff = p->current_time - q->current_time;
	if (diff == 0) {
		diff = p->cpu_burst_time - q->cpu_burst_time;
		if (diff == 0) {
			diff = p->io_time - q->io_time;
			if (diff == 0) {
				diff = p->initial_arrive_time - q->initial_arrive_time;
				if (diff == 0) {
					return strcmp(p->proc_id,q->proc_id);
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

void resize(process** processes, int* capacity) { // increases array size by 32
	*capacity += 4;
	*processes = realloc(*processes, (*capacity) * (sizeof(process)));
}

void freeProcesses(process** processes, int n) { // frees memory of words
	int i;
	for (i = 0; i < n; i++) {
		free((*processes)[i].proc_id);
		(*processes)[i].proc_id = NULL;
	}
	free(*processes);
}

void fileParser(process** processes, int* n, const char** arg1) { // parses file into process struct
	FILE* inputFile = NULL;
	inputFile = fopen(*arg1,"r");
	if (inputFile == NULL) error();
	int capacity = 4;
	*processes = (process*)calloc(capacity,sizeof(process));
	char buffer[80];
	memset(buffer,'\0',80);
	int i,j;
	while (fgets(buffer,80,inputFile) != NULL) { // reads line from file
		if (buffer[0] != '#' && buffer[0] != ' ') { // checks to see if line is non-commented
			if (*n >= capacity) resize(processes,&capacity); // resizes array if necessary
			char buffer2[80];
			memset(buffer2,'\0',80);
			for (i = 0, j = 0; j < 80; i++, j++) { // parses for proc-id
				if (buffer[i] != '|') buffer2[j] = buffer[i];
				else {
					i++;
					(*processes)[*n].proc_id = (char*)calloc(80,sizeof(char));
					strcpy((*processes)[*n].proc_id,buffer2);
					break;
				} 
			}
			memset(buffer2,'\0',80);
			for (j = 0; j < 80; i++, j++) { // parses for initial arrival time
				if (buffer[i] != '|') buffer2[j] = buffer[i];
				else {
					i++;
					(*processes)[*n].initial_arrive_time = atoi(buffer2);
					(*processes)[*n].current_time = (*processes)[*n].initial_arrive_time;
					(*processes)[*n].state = READY;
					break;
				} 
			}
			memset(buffer2,'\0',80);
			for (j = 0; j < 80; i++, j++) { // paraes for cpu-burst-time
				if (buffer[i] != '|') buffer2[j] = buffer[i];
				else {
					i++;
					(*processes)[*n].cpu_burst_time = atoi(buffer2);
					break;
				} 
			}
			memset(buffer2,'\0',80);
			for (j = 0; j < 80; i++, j++) { // parses for num-bursts
				if (buffer[i] != '|') buffer2[j] = buffer[i];
				else {
					i++;
					(*processes)[*n].num_bursts = atoi(buffer2);
					break;
				} 
			}
			memset(buffer2,'\0',80);
			for (j = 0; j < 80; i++, j++) { // parses for io-time
				if (buffer[i] != '\0') buffer2[j] = buffer[i];
				else {
					i++;
					(*processes)[*n].io_time = atoi(buffer2);
					break;
				} 
			}
			memset(buffer,'\0',80);
			(*n)++;
		}
	}
	#ifdef DEBUG_MODE
		printf("\narray capacity = %d. procceses = %d.\n",capacity,*n);
		printProcesses(processes,*n);
	#endif
	fclose(inputFile);
}

void FCFS_alg(process** processes, int* n, int t_cs, int* context_swtiches, int* preemptions) { // First Come First Server Alg
	int i, real_t = 0, change = 0, terminated = 0;
	qsort(*processes,*n,sizeof(process),comparator);
	#ifdef DEBUG_MODE
		printf("\n");
		printProcesses(processes,*n);
	#endif
	while (terminated != *n) {
		for (i = 0; i < *n - terminated; i++) {
			if ((*processes)[i].current_time == real_t && (*processes)[i].state == RUNNING) { // if proccess is RUNNING
				printf("time %dms: Process %s starts using the CPU\n",real_t,(*processes)[i].proc_id);
				fflush(stdout);
				(*context_swtiches)++;
				(*processes)[i].current_time += (*processes)[i].io_time;
				(*processes)[i].state = BLOCKED;
				change = 1;
			}
			if ((*processes)[i].current_time == real_t && (*processes)[i].state == BLOCKED) { // if prcoess is BLOCKED
				if ((*processes)[i].num_bursts == 0) {
					printf("time %dms: Process %s is terminated \n",real_t,(*processes)[i].proc_id);
					fflush(stdout);
				} else {
					printf("time %dms: Process %s is finishes using the CPU\n",real_t,(*processes)[i].proc_id);
					fflush(stdout);
				}
				(*processes)[i].current_time += (t_cs / 2);
				(*processes)[i].state = CONTEXT_SWITCH;
				change = 1;
			}
			if ((*processes)[i].current_time == real_t && (*processes)[i].state == CONTEXT_SWITCH) {
				if ((*processes)[i].num_bursts == 0) {
					(*processes)[i].state = TERMINATED;
					terminated++;
				} else {
					printf("time %dms: Process %s starts performing I/O\n",real_t,(*processes)[i].proc_id);
					fflush(stdout);
					(*processes)[i].state = READY;
				}
				change = 1;
			}
			if ((*processes)[i].current_time == real_t && (*processes)[i].state == READY) { // if proccess is READY
				if ((*processes)[i].current_time == (*processes)[i].initial_arrive_time) {
					printf("time %dms: Process %s arrives\n",real_t,(*processes)[i].proc_id);
					fflush(stdout);
				} else {
					printf("time %dms: Process %s finishes performing I/O\n",real_t,(*processes)[i].proc_id);
					fflush(stdout);
				}
				(*processes)[i].current_time += (*processes)[i].cpu_burst_time;
				((*processes)[i].num_bursts)--;
				(*processes)[i].state = RUNNING;
				change = 1;
			}
		}
		if (change) {
			change = 0;
			qsort(*processes,*n,sizeof(process),comparator);
			#ifdef DEBUG_MODE
				if (terminated < *n) {
					printProcesses(processes,*n - terminated);
				}
			#endif
		}
		if (real_t % 1000000 == 0) printf("time %dms:\n",real_t);
		if (terminated == *n) break;
		real_t++;
	}
	printf("time %dms: Simulator ended for FCFS\n",real_t + (t_cs / 2));
	fflush(stdout);
	
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
   printf("time %dms: Simulator started for FCFS\n",0);
   fflush(stdout);
   fileParser(&processes, &n, &argv[1]);
   int context_switches[n];
   int preemptions[n];
   
   FCFS_alg(&processes,&n,t_cs,&context_switches[0],&preemptions[0]);
   SRT();
   RR(rr_add);
   freeProcesses(&processes,n);
	return EXIT_SUCCESS;
}
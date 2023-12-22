#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Headers as needed

typedef enum {false, true} bool;        // Allows boolean types in C

/* Defines a job struct */
typedef struct Process {
    uint32_t A;                         // A: Arrival time of the process
    uint32_t B;                         // B: Upper Bound of CPU burst times of the given random integer list
    uint32_t C;                         // C: Total CPU time required
    uint32_t M;                         // M: Multiplier of CPU burst time
    uint32_t processID;                 // The process ID given upon input read

    uint8_t status;                     // 0 is unstarted, 1 is ready, 2 is running, 3 is blocked, 4 is terminated

    int32_t finishingTime;              // The cycle when the the process finishes (initially -1)
    uint32_t currentCPUTimeRun;         // The amount of time the process has already run (time in running state)
    uint32_t currentIOBlockedTime;      // The amount of time the process has been IO blocked (time in blocked state)
    uint32_t currentWaitingTime;        // The amount of time spent waiting to be run (time in ready state)

    uint32_t IOBurst;                   // The amount of time until the process finishes being blocked
    uint32_t CPUBurst;                  // The CPU availability of the process (has to be > 1 to move to running)

    int32_t quantum;                    // Used for schedulers that utilise pre-emption

    bool isFirstTimeRunning;            // Used to check when to calculate the CPU burst when it hits running mode

    struct Process* nextInBlockedList;  // A pointer to the next process available in the blocked list
    struct Process* nextInReadyQueue;   // A pointer to the next process available in the ready queue
    struct Process* nextInReadySuspendedQueue; // A pointer to the next process available in the ready suspended queue
} _process;


uint32_t CURRENT_CYCLE = 0;             // The current cycle that each process is on
uint32_t TOTAL_CREATED_PROCESSES = 0;   // The total number of processes constructed
uint32_t TOTAL_STARTED_PROCESSES = 0;   // The total number of processes that have started being simulated
uint32_t TOTAL_FINISHED_PROCESSES = 0;  // The total number of processes that have finished running
uint32_t TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED = 0; // The total cycles in the blocked state

const char* RANDOM_NUMBER_FILE_NAME= "random-numbers";
const uint32_t SEED_VALUE = 200;  // Seed value for reading from file

// Additional variables as needed


/**
 * Reads a random non-negative integer X from a file with a given line named random-numbers (in the current directory)
 */
uint32_t getRandNumFromFile(uint32_t line, FILE* random_num_file_ptr){
    uint32_t end, loop;
    char str[512];

    rewind(random_num_file_ptr); // reset to be beginning
    for(end = loop = 0;loop<line;++loop){
        if(0==fgets(str, sizeof(str), random_num_file_ptr)){ //include '\n'
            end = 1;  //can't input (EOF)
            break;
        }
    }
    if(!end) {
        return (uint32_t) atoi(str);
    }

    // fail-safe return
    return (uint32_t) 1804289383;
}



/**
 * Reads a random non-negative integer X from a file named random-numbers.
 * Returns the CPU Burst: : 1 + (random-number-from-file % upper_bound)
 */
uint32_t randomOS(uint32_t upper_bound, uint32_t process_indx, FILE* random_num_file_ptr)
{
    char str[20];
    
    uint32_t unsigned_rand_int = (uint32_t) getRandNumFromFile(SEED_VALUE+process_indx, random_num_file_ptr);
    uint32_t returnValue = 1 + (unsigned_rand_int % upper_bound);

    return returnValue;
} 

_process init_process(int A, int B, int C, int M, int processId) {
    _process newprocess = {
        A = A,
        B = B,
        C = C,
        M = M
    };

    newprocess.processID = processId;
    newprocess.quantum = 2;  // only used by Round Robin

    return newprocess;
}


/********************* SOME PRINTING HELPERS *********************/


/**
 * Prints to standard output the original input
 * proccessList is the original processes inputted (in array form)
 */
void printStart(_process proccessList[])
{
    printf("The original input was: %i", TOTAL_CREATED_PROCESSES);

    uint32_t i = 0;
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        printf(" ( %i %i %i %i)", proccessList[i].A, proccessList[i].B,
               proccessList[i].C, proccessList[i].M);
    }
    printf("\n");
} 

/**
 * Prints to standard output the final output
 * finishedProccessList is the terminated processes (in array form) in the order they each finished in.
 */
void printFinal(_process finishedProccessList[])
{
    printf("The (sorted) input is: %i", TOTAL_CREATED_PROCESSES);

    uint32_t i = 0;
    for (; i < TOTAL_FINISHED_PROCESSES; ++i)
    {
        printf(" ( %i %i %i %i)", finishedProccessList[i].A, finishedProccessList[i].B,
               finishedProccessList[i].C, finishedProccessList[i].M);
    }
    printf("\n");
} // End of the print final function

/**
 * Prints out specifics for each process.
 * @param proccessList The original processes inputted, in array form
 */
void printProcessSpecifics(_process proccessList[])
{
    uint32_t i = 0;
    printf("\n");
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        printf("Process %i:\n", proccessList[i].processID);
        printf("\t(A,B,C,M) = (%i,%i,%i,%i)\n", proccessList[i].A, proccessList[i].B,
               proccessList[i].C, proccessList[i].M);
        printf("\tFinishing time: %i\n", proccessList[i].finishingTime);
        printf("\tTurnaround time: %i\n", proccessList[i].finishingTime - proccessList[i].A);
        printf("\tI/O time: %i\n", proccessList[i].currentIOBlockedTime);
        printf("\tWaiting time: %i\n", proccessList[i].currentWaitingTime);
        printf("\n");
    }
} // End of the print process specifics function

/**
 * Prints out the summary data
 * proccessList The original processes inputted, in array form
 */
void printSummaryData(_process proccessList[])
{
    uint32_t i = 0;
    double total_amount_of_time_utilizing_cpu = 0.0;
    double total_amount_of_time_io_blocked = 0.0;
    double total_amount_of_time_spent_waiting = 0.0;
    double total_turnaround_time = 0.0;
    uint32_t final_finishing_time = CURRENT_CYCLE - 1;
    for (; i < TOTAL_CREATED_PROCESSES; ++i)
    {
        total_amount_of_time_utilizing_cpu += proccessList[i].currentCPUTimeRun;
        total_amount_of_time_io_blocked += proccessList[i].currentIOBlockedTime;
        total_amount_of_time_spent_waiting += proccessList[i].currentWaitingTime;
        total_turnaround_time += (proccessList[i].finishingTime - proccessList[i].A);
    }

    // Calculates the CPU utilisation
    double cpu_util = total_amount_of_time_utilizing_cpu / final_finishing_time;

    // Calculates the IO utilisation
    double io_util = (double) TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED / final_finishing_time;

    // Calculates the throughput (Number of processes over the final finishing time times 100)
    double throughput =  100 * ((double) TOTAL_CREATED_PROCESSES/ final_finishing_time);

    // Calculates the average turnaround time
    double avg_turnaround_time = total_turnaround_time / TOTAL_CREATED_PROCESSES;

    // Calculates the average waiting time
    double avg_waiting_time = total_amount_of_time_spent_waiting / TOTAL_CREATED_PROCESSES;

    printf("Summary Data:\n");
    printf("\tFinishing time: %i\n", CURRENT_CYCLE - 1);
    printf("\tCPU Utilisation: %6f\n", cpu_util);
    printf("\tI/O Utilisation: %6f\n", io_util);
    printf("\tThroughput: %6f processes per hundred cycles\n", throughput);
    printf("\tAverage turnaround time: %6f\n", avg_turnaround_time);
    printf("\tAverage waiting time: %6f\n", avg_waiting_time);
} // End of the print summary data function


// uint32_t A;                         // A: Arrival time of the process
// uint32_t B;                         // B: Upper Bound of CPU burst times of the given random integer list
// uint32_t C;                         // C: Total CPU time required
// uint32_t M;                         // M: Multiplier of CPU burst time

// Turn around time = completion time - time of arrival
// Response time = first run time - time of arrival
/*
Reset functions to so when running another scheudler we can get accurate
results
*/
void resetProccesCounters(){
    CURRENT_CYCLE = 0;
    TOTAL_STARTED_PROCESSES = 0;
    TOTAL_FINISHED_PROCESSES = 0;
    TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED = 0;
}
/*
Reset functions to so when running another scheudler we can get accurate
results
*/
void resetProccessList(_process processList[]){
    for(int i =0; i < TOTAL_CREATED_PROCESSES; i++){
        processList[i].currentCPUTimeRun = 0;
        processList[i].currentIOBlockedTime = 0;
        processList[i].currentWaitingTime = 0;
        processList[i].finishingTime = 0;
        processList[i].CPUBurst = 0;
        processList[i].IOBurst = 0;
        processList[i].status = 0;
        processList[i].isFirstTimeRunning = 0;
        processList[i].nextInBlockedList = NULL;
        processList[i].nextInReadyQueue = NULL;
    }
}

//crtl + shift + L

void readyProccessFunc(_process **readyProccess, _process *newestReadyProccess){
    _process *curProccess = *readyProccess;
    newestReadyProccess->status = 1;

    if (curProccess == NULL) {
        *readyProccess = newestReadyProccess;
        return;
    }
    if (newestReadyProccess->A < curProccess->A ||
        (newestReadyProccess->A == curProccess->A && newestReadyProccess->processID < curProccess->processID)) {
        newestReadyProccess->nextInReadyQueue = *readyProccess;
        *readyProccess = newestReadyProccess;
        return;
    }
    while (curProccess->nextInReadyQueue != NULL) {
        if (newestReadyProccess->A < curProccess->nextInReadyQueue->A ||
            (newestReadyProccess->A == curProccess->nextInReadyQueue->A && newestReadyProccess->processID < curProccess->nextInReadyQueue->processID)) {
            newestReadyProccess->nextInReadyQueue = curProccess->nextInReadyQueue;
            curProccess->nextInReadyQueue = newestReadyProccess;
            return;
        }

        curProccess = curProccess->nextInReadyQueue;
    }

   
    curProccess->nextInReadyQueue = newestReadyProccess;

}

void blockProccessFunc(_process **blockedProccess, _process *newestBlockedProccess) { 
    _process *curProcess = *blockedProccess;
    newestBlockedProccess->status = 3;

    if (*blockedProccess == NULL) {
        *blockedProccess = newestBlockedProccess;
        return;
    }
    if (newestBlockedProccess->IOBurst < curProcess->IOBurst) {
        newestBlockedProccess->nextInBlockedList = *blockedProccess;
        *blockedProccess = newestBlockedProccess;
        return;
    }

    curProcess = *blockedProccess;

    while (curProcess->nextInBlockedList != NULL) {

        if (newestBlockedProccess->IOBurst < curProcess->nextInBlockedList->IOBurst) {
            newestBlockedProccess->nextInBlockedList = curProcess->nextInBlockedList;
            curProcess->nextInBlockedList = newestBlockedProccess;
            return;
        }

        curProcess = curProcess->nextInBlockedList;
    }

    curProcess->nextInBlockedList = newestBlockedProccess;
}

void exeProcessFunc(_process *curProccess, FILE* infile){
    uint32_t burst, timeDif;
    if (curProccess->CPUBurst == 0) {
        timeDif = curProccess->C - curProccess->currentCPUTimeRun;

        burst = randomOS(curProccess->B, curProccess->processID, infile);
 
        if (timeDif < burst) {
            curProccess->CPUBurst = timeDif;
        } else {
            curProccess->CPUBurst = burst;
            curProccess->IOBurst = burst * curProccess->M;
        }
    }

    curProccess->nextInBlockedList = NULL;
    curProccess->nextInReadyQueue = NULL;
    curProccess->status = 2;

    if (curProccess->isFirstTimeRunning) {
        TOTAL_STARTED_PROCESSES++;
        curProccess->isFirstTimeRunning = false;
    }
}

void readyProccessListFunc(_process processList[], _process **readyProc) {
    for (int i = 0; i < TOTAL_CREATED_PROCESSES; i++) {
        if (processList[i].A == CURRENT_CYCLE) {
            readyProccessFunc(readyProc, &processList[i]);
        }
    }
}


void fcfsScheduler(_process proccessList[], _process finishedProccessList[], FILE *infile){
    _process *runningProc = NULL, *readyProccess = NULL, *blockedProccess = NULL, *curProcess;
    readyProccessListFunc(proccessList, &readyProccess);
    runningProc = readyProccess;
    readyProccess = runningProc->nextInReadyQueue;
    exeProcessFunc(runningProc, infile);


    while (TOTAL_FINISHED_PROCESSES != TOTAL_CREATED_PROCESSES) {
        CURRENT_CYCLE++;
        if (runningProc != NULL && (runningProc->status == 3 
        || runningProc->status == 4)) {
            if (readyProccess != NULL) {
                runningProc = readyProccess;
                readyProccess= readyProccess->nextInReadyQueue;
                exeProcessFunc(runningProc, infile);
            } else {

                runningProc = NULL;
            }
        } else if (runningProc == NULL && readyProccess != NULL) {
            
            runningProc = readyProccess;
            readyProccess = runningProc->nextInReadyQueue;
            exeProcessFunc(runningProc, infile);
        }

        
        curProcess = readyProccess;
        while (curProcess != NULL) {
            curProcess->currentWaitingTime++;
            curProcess = curProcess->nextInReadyQueue;
        }

        
        if (blockedProccess != NULL) {
            TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED++;
            curProcess = blockedProccess;

            
            while (curProcess != NULL) {
                curProcess->IOBurst--;
                curProcess = curProcess->nextInBlockedList;
            }

            
            while (blockedProccess != NULL && blockedProccess->IOBurst == 0) {
                readyProccessFunc(&readyProccess, blockedProccess);
                blockedProccess = blockedProccess->nextInBlockedList;
            }
        }

        readyProccessListFunc(proccessList, &readyProccess);

        
        if (runningProc == NULL) {
            continue;
        }

        runningProc->currentCPUTimeRun++;
        runningProc->CPUBurst--;

        if (runningProc->currentCPUTimeRun == runningProc->C) {
            
            runningProc->status = 4;
            runningProc->finishingTime = CURRENT_CYCLE;
            finishedProccessList[TOTAL_FINISHED_PROCESSES] = *runningProc;
            TOTAL_FINISHED_PROCESSES++;
        } else if (runningProc->CPUBurst == 0) {
            blockProccessFunc(&blockedProccess, runningProc);
        }
    }
    CURRENT_CYCLE++;
}


void rrScheduler(_process proccessList[], _process finishedProccessList[], FILE *infile) {
    _process *runningProc = NULL, *readyProccess = NULL, *blockedProccess = NULL, *curProcess;
    uint32_t quantum, quantum_counter; 

    readyProccessListFunc(proccessList, &readyProccess);

    runningProc = readyProccess;
    readyProccess = runningProc->nextInReadyQueue;
    exeProcessFunc(runningProc, infile);

    quantum = runningProc->quantum;
    quantum_counter = quantum;

    while (TOTAL_FINISHED_PROCESSES != TOTAL_CREATED_PROCESSES) {
        CURRENT_CYCLE++;
        quantum_counter--;

        curProcess = readyProccess;
        while (curProcess != NULL) {
            curProcess->currentWaitingTime++;
            curProcess = curProcess->nextInReadyQueue;
        }

        readyProccessListFunc(proccessList, &readyProccess);

        if (blockedProccess != NULL) {
            TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED++;
            curProcess = blockedProccess;

            
            while (curProcess != NULL) {
                curProcess->IOBurst--;
                curProcess->currentIOBlockedTime++;
                curProcess = curProcess->nextInBlockedList;
            }

            
            while (blockedProccess != NULL && blockedProccess->IOBurst == 0) {
                readyProccessFunc(&readyProccess, blockedProccess);
                blockedProccess = blockedProccess->nextInBlockedList;
            }
        }

        
        if (runningProc != NULL) {
            runningProc->currentCPUTimeRun++;
            runningProc->CPUBurst--;

            if (runningProc->currentCPUTimeRun == runningProc->C) {
                
                runningProc->status = 4;
                runningProc->finishingTime = CURRENT_CYCLE;
                finishedProccessList[TOTAL_FINISHED_PROCESSES] = *runningProc;
                runningProc = NULL;

                TOTAL_FINISHED_PROCESSES++;
            } else if (runningProc->CPUBurst == 0) {
                blockProccessFunc(&blockedProccess, runningProc);
                runningProc = NULL;
            }
        }

        
        if (quantum_counter == 0 || runningProc == NULL) {
            quantum_counter = quantum;

            if (runningProc != NULL) {
                readyProccessFunc(&readyProccess, runningProc);
                runningProc = NULL;
            }

            if (readyProccess != NULL) {
                runningProc = readyProccess;
                readyProccess = readyProccess->nextInReadyQueue;
                exeProcessFunc(runningProc, infile);
            }
        }
    }

    CURRENT_CYCLE++;
}

void sjfScheduler(_process proccessList[], _process finishedProccessList[], FILE *infile) {
    _process *runningProc = NULL, *readyProccess = NULL, *blockedProcess = NULL, *curProcess, *shortestJob;

    readyProccessListFunc(proccessList, &readyProccess);

    runningProc = readyProccess;
    readyProccess = runningProc->nextInReadyQueue;
    exeProcessFunc(runningProc, infile);

    while (TOTAL_FINISHED_PROCESSES != TOTAL_CREATED_PROCESSES) {
        CURRENT_CYCLE++;

        curProcess = readyProccess;
        while (curProcess != NULL) {
            curProcess->currentWaitingTime++;
            curProcess = curProcess->nextInReadyQueue;
        }

        readyProccessListFunc(proccessList, &readyProccess);

        if (blockedProcess != NULL) {
            TOTAL_NUMBER_OF_CYCLES_SPENT_BLOCKED++;
            curProcess = blockedProcess;

            while (curProcess != NULL) {
                curProcess->IOBurst--;
                curProcess->currentIOBlockedTime++;
                curProcess = curProcess->nextInBlockedList;
            }

            while (blockedProcess != NULL && blockedProcess->IOBurst == 0) {
                readyProccessFunc(&readyProccess, blockedProcess);
                blockedProcess = blockedProcess->nextInBlockedList;
            }
        }

        if (runningProc != NULL) {
            runningProc->currentCPUTimeRun++;
            runningProc->CPUBurst--;

            if (runningProc->currentCPUTimeRun == runningProc->C) {

                runningProc->status = 4;
                runningProc->finishingTime = CURRENT_CYCLE;
                finishedProccessList[TOTAL_FINISHED_PROCESSES] = *runningProc;
                runningProc = NULL;

                TOTAL_FINISHED_PROCESSES++;
            } else if (runningProc->CPUBurst == 0) {
                blockProccessFunc(&blockedProcess,runningProc);
                runningProc = NULL;
            }
        }

        if (runningProc == NULL && readyProccess != NULL) {
            shortestJob = readyProccess;
            curProcess = readyProccess->nextInReadyQueue;

            while (curProcess != NULL) {
                if (curProcess->CPUBurst < shortestJob->CPUBurst || 
                    (curProcess->CPUBurst == shortestJob->CPUBurst && curProcess->A < shortestJob->A)) {
                    shortestJob = curProcess;
                }

                curProcess = curProcess->nextInReadyQueue;
            }
            
            if (shortestJob == readyProccess) {
                readyProccess = readyProccess->nextInReadyQueue;
            } 
            else {
                curProcess = readyProccess;
            
                while (curProcess->nextInReadyQueue != shortestJob) {
                    curProcess = curProcess->nextInReadyQueue;
                }

                curProcess->nextInReadyQueue = shortestJob->nextInReadyQueue;
            }

            runningProc = shortestJob;
            exeProcessFunc(runningProc, infile);
        }
    }

    CURRENT_CYCLE++;
}



int get_process_count(char *inputLine) {
    char tmpString[256], *token; const char delim[2] = " ";
    strcpy(tmpString, inputLine);
    token = strtok(tmpString, delim);
    return atoi(token);
}

_process* parseLine(char *inputLine, uint32_t *numProccess) {
    char *token; const char delim[4] = "() ";
    _process *processList;
    int val[4];

    *numProccess = get_process_count(inputLine);
    processList = (_process*)malloc(*numProccess * sizeof(_process));

    token = strtok(inputLine, " ");
    token = strtok(NULL, delim);

    for (int i = 0; i < *numProccess; i++) {
        for (int j = 0; j < 4; j++) {
            if (token != NULL) {
                val[j] = atoi(token);
            } else {
                return NULL;
            }
            token = strtok(NULL, delim);
        }
        processList[i] = init_process(val[0], val[1], val[2], val[3], i);
    }
    return processList;
}

_process* parseFile(char *infile, uint32_t *numProcess) {
    char *inputLine;
    size_t length = 0;
    FILE *processFile = fopen(infile, "r");

    if (processFile == NULL) {
        return NULL;
    }
    getline(&inputLine, &length, processFile);
    return parseLine(inputLine, numProcess);
}

/**
 * The magic starts from here
 */
int main(int argc, char *argv[])
{
    FILE *infile = fopen(RANDOM_NUMBER_FILE_NAME, "r");
    _process *processList, *finishedProccessList;

    printf("%s\n",argv[1]);
    processList = parseFile(argv[1], &TOTAL_CREATED_PROCESSES);
    finishedProccessList = (_process*)malloc(TOTAL_CREATED_PROCESSES * sizeof(_process));


    /*
    run each scheduler on our input test then print results and reset values to run 
    different scheduler
    */
    for (int i = 0; i < 3; i++) {
        resetProccesCounters();
        resetProccessList(processList);

        if (i == 0) {
            printf("START OF FIRST COME FIRST SERVE\n");
            fcfsScheduler(processList, finishedProccessList, infile);
        } 
        else if (i == 1) {
            printf("\nSTART OF ROUND ROBIN\n");
            rrScheduler(processList, finishedProccessList, infile);
        } 
        else {
            printf("\nSTART OF SHORTEST JOB FIRST\n");
            sjfScheduler(processList, finishedProccessList, infile);
        }
        
        printStart(processList);
        printFinal(finishedProccessList);
        printProcessSpecifics(processList);
        printSummaryData(processList);
    }

    fclose(infile);
    return 0;
} 
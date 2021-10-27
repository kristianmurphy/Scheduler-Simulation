#include <iostream>
#include <string>
using namespace std;
// Include basic headers for c++ functionality

class PCB; // Class prototype for PCB (Process Control Block)
void simulation(string scheduler); // Prototype for simulation function - passes input string scheduler, which could be "SJF", "FCFS", or "MLFQ"



class PCB // Class PCB to keep track of information for each process - Data members:
{
public: // Public data of PCB class includes the accessor and mutator functions of the class
    PCB* getLast();  // Used to quickly access the end of the list in each state going through the pcbPointers of each PCB
    PCB* getShortest(); // Used to find the shortest process in the list
    PCB() { newProcess(); }  // Default value constructor using new process function - is not new state yet (until process ID and data are assigned)
    PCB(int num, int *array, int arraysize);  // Explicit value constructor assigns process ID and pointer to process data array, setting the process in new state
    
    int getLength(); // Get the length of list starting from this PCB and going through the pcbPointers

    void newProcess();  // Called by constructor to initialize PCB default values
    void admitProcess(PCB** ready);  // After Explicit value constructor is called, the process is admitted to the ready queue
    void runProcess(PCB** ready, PCB** running, int time);  // Runs process from ready state
    void waitProcess(PCB** running, PCB** waiting);  // Sends the process to waiting list from running state
    void readyProcess(PCB** waiting, PCB** ready);  // Sends the process to ready queue from waiting state
    void terminateProcess(PCB** running, PCB** terminated, int time);  // Terminates process from running state

    void cpuManage(PCB** running, PCB** waiting, PCB** terminated, int time);  // Determines whether process in cpu should go to waiting or terminated states if at the end of cpu burst
    void waitManage(PCB** waiting, PCB** ready);  // Determines whether process in waiting list should go to ready queue
    
    void waitTiming() { waitingTime++; if (pcbPointer != NULL) { pcbPointer->waitTiming(); } }
    void cpuTiming() { cpuTime++; if (pcbPointer != NULL) { pcbPointer->cpuTiming(); } }
    void ioTiming() { ioTime++; if (pcbPointer != NULL) { pcbPointer->ioTiming(); } }
    
    void printStatus(); // Prints the status of process to indicate its progress in CPU, IO, Ready queue, or termination
    void printTable(PCB* head, int id); // Prints the final values in each process, in order P1 to Px (Called recursively uusing head pointer to scan for numeric order in terminated list)

    void incrementPC();

private: // Private data of PCB class includes the variables holding information to each process
    string processState; // Keep track of which state this process should be in
    int processID;      // Keep track of process number (P1-8)
    PCB* pcbPointer;   // Pointer to next PCB - Used in each state to keep track of the order of PCBs
    int* processData; // Pointer to the first CPU burst of the process data (CPU and IO burst) array
    int dataSize;    // A variable saved with the process data to remember the size of its array
    int dataIndex;  // Keep track of which burst the process is on starting with 0
    int dataTime;
    int waitingTime;         // Accumulate 1 for every time unit spent in waiting queue
    int cpuTime;            // Accumulate 1 for every time unit spent in CPU
    int ioTime;            // Accumulate 1 for every time unit spent in I/O
    int responseTime;     // Record the first time of execution in each process
    int terminationTime; // Record the time that the process is terminated
};



void PCB::newProcess() {
    pcbPointer = NULL;        // Initialize pointer as null
    waitingTime = 0;         // Accumulate 1 for every time unit spent in waiting queue
    cpuTime = 0;            // Accumulate 1 for every time unit spent in CPU
    ioTime = 0;            // Accumulate 1 for every time unit spent in I/O
    dataIndex = -1;       // Start program counter at -1 since execution hasn't yet started (sets to 0 when start, then increments)
    dataTime = 0;
    responseTime = -1;  // Start response time at -1 to indicate that it has not been set yet - upon first execution of the process this will be set to the time of execution start
    processState = ""; // Start process state at "" (empty string) to indicate that it has not been fully created yet - upon being given a processID and data, it can be a new process
}

PCB::PCB(int num, int *array, int arraysize) {
    newProcess();             // Initialize process with program counter = 0
    processState = "new";    // Initialize a process in new state
    processID = num;        // Initialize process with given process ID (P1-8)
    processData = array;   // Assign the input array to the data pointer
    dataSize = arraysize; // Keep the passed function argument of array size as var
}

PCB* PCB::getLast() { // Calls recursively on itself to return the very last PCB pointer in a list, calling first on the head of list
    if (pcbPointer == NULL) { return this; } // If at the last PCB of the list, the next pointer will be null (and if the list is only 1 PCB)
    else { return pcbPointer->getLast(); }  // Otherwise call recursively on the next PCB pointer, eventually returning the last one all the way back
}

PCB* PCB::getShortest() {  // Used for SJF scheduling to find the shortest job in the ready queue, by calling recursively starting with the head of the list
    if (pcbPointer == NULL) { return this; }  // If the next PCB pointer is NULL, which means the end of the list, (or list is only 1 PCB) return the current PCB
    else {  // Otherwise compare the next PCB pointer's next CPU burst to the current PCB's next CPU burst which is found through the processData array and using the index+1 for next burst
        if (pcbPointer->getShortest()->processData[pcbPointer->getShortest()->dataIndex + 1] < processData[dataIndex + 1]) { return pcbPointer->getShortest(); }
        else { return this; }  // If the current PCB's CPU burst is greater than or equal to the next (lower) PCB's CPU burst, return this pointer
    }
}

void PCB::admitProcess(PCB** ready) { // Used to admit processes in the new state into the ready state at the beginning of the simulation (since all processes arrive at t=0)
    if ((processState == "new") && ready != NULL) { // First check if the PCB's process state is at new and that the pointer to ready list pointer is not null
        processState = "ready";  // If so, set the current process state to ready (If not, then the *ready list pointer must not have been found from the **ready double pointer)
        if (*ready == NULL) { *ready = this; }  // If the ready list pointer is null (empty list), set the head of the list to this process
        else { (*ready)->getLast()->pcbPointer = this; } // If the ready list is not empty, then get the very last PCB in that list and then add the current PCB onto the end of its next pointer
    }  // If the double pointer was not found or the process was not in the new state (must be in new to be 'admitted' to ready)
    else { cout << "Could not admit process " << processID << " - [" << processState << "]" << endl; } // Print error message
}

void PCB::runProcess(PCB** ready, PCB** running, int time) {  // moves process from ready list to running, preempting 
    if (processState == "ready") {  // First check if process is in ready state (process can only go into running state from ready)
        processState = "running";  // Set current process state to running
        if (*ready == this) {     // If the head of the ready list is the current PCB
            *ready = pcbPointer; // set the head of ready list to this PCB's next pointer (if null, then list is empty)
        }
        else { // Otherwise (if the current PCB is not the head of the ready list)
            PCB* temp = *ready;  // Use temporary pointer to go through ready list to find the preceding PCB to the current one
            if (temp->pcbPointer != NULL) {  // If the temporary PCB's pointer to next PCB (in ready list) is valid:
                while (temp->pcbPointer != this) { temp = temp->pcbPointer; } // Go through each of the descendants of the temporary PCB pointer
                temp->pcbPointer = temp->pcbPointer->pcbPointer; // After using while loop to find parent of running PCB, set its next PCB pointer to the current PCB's next pointer to mend the list
            }
        }
        *running = this;    // Set the running pointer to the current PCB
        pcbPointer = NULL; // Set the next PCB pointer to NULL since there is only 1 process running at a time (no 'next' pointers in running)
        if (responseTime == -1) { responseTime = time; } // set response time when process is ran (if first time -1)
    } // Otherwise:  (if the process state was not "ready" to begin with)
    else { cout << "Could not run process " << processID << " - [" << processState << "]" << endl; } // Print error message
}

void PCB::waitProcess(PCB** running, PCB** waiting) { // Moves current process from running to waiting list
    if (processState == "running") {  // Check if the current PCB's process state is running (can not go to waiting list unless currently running)
        processState = "waiting";    //  If so, update current PCB's process state to waiting
        if (*waiting == NULL) { *waiting = this; }  // If the waiting list head pointer is NULL (list is empty), set the pointer to head of list to the current process
        else { (*waiting)->getLast()->pcbPointer = this; } // If the waiting list is not empty, get the last PCB of the list and set its next pointer to the current process
        *running = NULL;    // Now that the process has been moved to the waiting list, set the running list to NULL (empty)
        pcbPointer = NULL; // Make sure that the pointer to next PCB is set to NULL since the current process is at the very end of the waiting list
    } // Otherwise:  (if the process state was not "running" to begin with)
    else { cout << "Could not move process " << processID << " to waiting queue - " << "[" << processState << "]" << endl; } // Print error message
}

void PCB::readyProcess(PCB** waiting, PCB** ready) {  // Moves process from waiting list to ready queue
    if (processState == "waiting") {  // Check if the current PCB's proccess state is waiting (can not go to ready queue unless the currently in waiting list)
        processState = "ready";      // If so, update the current PCB's process state to ready
        if (*waiting == this) { *waiting = pcbPointer; }    // If the head of the waiting list is the current process: set the new head of the waiting list to the pointer to next PCB (if NULL, empty list)
        else {  // If the current process is not the head of the waiting list:
            PCB* temp = *waiting;  // Set a temporary pointer to PCB as the head of the waiting list
            if (temp->pcbPointer != NULL) {  // 
                while (temp->pcbPointer != this) { temp = temp->pcbPointer; }
                temp->pcbPointer = temp->pcbPointer->pcbPointer;
            }
        }
        if (*ready == NULL) { *ready = this; }
        else { (*ready)->getLast()->pcbPointer = this; }
        pcbPointer = NULL;
    }
    else { cout << "Could not move process " << processID << " to ready list - " << "[" << processState << "]" << endl; }
}

void PCB::terminateProcess(PCB** running, PCB** terminated, int time) {
    if (processState == "running") {
        if (*terminated == NULL) { *terminated = this; }
        else { (*terminated)->getLast()->pcbPointer = this; }
        *running = NULL;
        processState = "terminated";
        terminationTime = time;
        pcbPointer = NULL;
    }
    else { cout << "Could not terminate process " << processID << " - [" << processState << "]" << endl; }
}

void PCB::incrementPC() {  // Increments the dataIndex, or program counter index in the array of process data (IO/CPU bursts)
    if (dataIndex + 1 < dataSize) {  // If the data index is less than the total size of the data array
        dataIndex++;  // Increment the data index to the next index in the data array
        dataTime += processData[dataIndex];  // Add to the process dataTime with the new processData index so that the total burst time can be saved
    }
}

void PCB::cpuManage(PCB** running, PCB** waiting, PCB** terminated, int time) {
    if (dataTime - ioTime - cpuTime == 0) {
        if (dataIndex+1 >= dataSize) { terminateProcess(running, terminated, time); }
        else { incrementPC();  waitProcess(running, waiting); }
    }
}

void PCB::waitManage(PCB** waiting, PCB** ready) {
    if (dataTime - ioTime - cpuTime == 0) {
        PCB* p = pcbPointer;
        readyProcess(waiting, ready);
        if (p != NULL) { p->waitManage(waiting, ready); }
    }
    else {
        if (pcbPointer != NULL) { pcbPointer->waitManage(waiting, ready); }
    }
}

void PCB::printStatus() {
    if (processState == "ready") {
        cout << "P" << processID << " [Next CPU burst: " << processData[dataIndex+1] << "]" << endl;
    }
    else if (processState == "running") {
        cout << "Running: P" << processID << endl;
    }
    else if (processState == "waiting") {
        cout << "P" << processID << " [Remaining IO burst: " << dataTime - (ioTime+cpuTime) << "]" << endl;
    }
    else if (processState == "terminated") {
        cout << "P" << processID << " Terminated at time: " << terminationTime << endl;
    }
    if (pcbPointer != NULL) {
        if (processState != pcbPointer->processState) { cout << "Error - (" << processID << ") " << processState << " != (" << pcbPointer->processID << ") " << processState << "!" << endl; }
        else {
            pcbPointer->printStatus();
        }
    }
}

void PCB::printTable(PCB* head, int id) { // At the end of simulation, print all details of processes in list (terminated) in order
    if (id < head->getLength()) {
        PCB* search = head;
        while (search != NULL && search->processID != id + 1) { search = search->pcbPointer; }
        if (search != NULL) { cout << "P" << search->processID << "    Tw = " << (search->waitingTime-1) << "    Ttr = " << (search->waitingTime-1 + search->ioTime + search->cpuTime) << "    Tr = " << search->responseTime << endl; }
        search->printTable(head, id + 1);
    }
    else if (id == head->getLength()) {
        float avgTw = 0, avgTtr = 0, avgTr = 0;
        PCB* i = head;
        while (i != NULL) {
            avgTw += i->waitingTime;
            avgTtr += i->waitingTime + i->ioTime + i->cpuTime;
            avgTr += i->responseTime;
            i = i->pcbPointer;
        }
        avgTw /= head->getLength();
        avgTtr /= head->getLength();
        avgTr /= head->getLength();
        cout << "Avg" << "   Tw = " << avgTw << "    Ttr = " << avgTtr << "    Tr = " << avgTr << endl;

    }

}

int PCB::getLength() {
    PCB* p = this;
    int count = 1;
    while (p != NULL) { p = p->pcbPointer; count++ ; }
    return count;
}



// Non-member functions: //

void programCounter(PCB* ready, PCB* running, PCB* waiting) {
    if (ready != NULL) { ready->waitTiming(); }
    if (running != NULL) { running->cpuTiming(); }
    if (waiting != NULL) { waiting->ioTiming(); }
}

void scheduleSJF(PCB** ready, PCB** running, PCB** waiting, PCB** terminated) {
    if (ready != NULL && running != NULL && waiting != NULL && terminated != NULL) {
        int idleTime = 0;
        for (int t = 0; t <= 1024; t++) {  // TIME loop - time units t
            //**order
            if (*waiting != NULL) {
                (*waiting)->waitManage(waiting, ready);
            }
            if (*running != NULL) {
                (*running)->cpuManage(running, waiting, terminated, t);
            }
            if (*running == NULL) {     // Check if no processes are running (no preemption)
                if (*ready != NULL) {  // If there is a process in ready queue:
                    
                    (*ready)->getShortest()->runProcess(ready, running, t);  // Run first process in ready queue
                    (*running)->incrementPC();
                    cout << "\nCurrent Execution time: " << t << endl;
                    (*running)->printStatus();
                    if (*ready != NULL) { cout << "Ready:" << endl; (*ready)->printStatus(); }
                    if (*waiting != NULL) { cout << "Waiting:" << endl; (*waiting)->printStatus(); }
                    if (*terminated != NULL) { cout << "Terminated:" << endl; (*terminated)->printStatus(); }


                }
            }

            if (*ready == NULL && *waiting == NULL && *running == NULL) { // If all queues are finished then print final table and end loop

                if (*terminated != NULL) {
                    cout << "\nTotal time: " << t << "  CPU Utilization: " << 100 * (1 - (idleTime / t)) << "%" << endl;
                    (*terminated)->printTable(*terminated, 0);
                }
                return;
            }
            else {
                if (*running == NULL) { idleTime++; } // Otherwise, if there is no process running then increment the idle time var
                programCounter(*ready, *running, *waiting); // Increment the waiting, cpu, and io times
            }

        }
    }
}

void scheduleFCFS(PCB** ready, PCB** running, PCB** waiting, PCB** terminated) {
    if(ready != NULL && running != NULL && waiting != NULL && terminated != NULL){
        int idleTime = 0;
        for (int t = 0; t <= 1024; t++) {  // TIME loop - time units t
         
            if (*waiting != NULL) {
                (*waiting)->waitManage(waiting, ready);
            }
            if (*running != NULL) {
                (*running)->cpuManage(running, waiting, terminated, t);
            }
            if (*running == NULL) {     // Check if no processes are running (no preemption)
                if (*ready != NULL) {  // If there is a process in ready queue:
                    (*ready)->incrementPC();
                    (*ready)->runProcess(ready, running, t);  // Run first process in ready queue

                    cout << "\nCurrent Execution time: " << t << endl;
                    (*running)->printStatus();
                    if (*ready != NULL) { cout << "Ready:" << endl; (*ready)->printStatus(); }
                    if (*waiting != NULL) { cout << "Waiting:" << endl; (*waiting)->printStatus(); }
                    if (*terminated != NULL) { cout << "Terminated:" << endl; (*terminated)->printStatus(); }


                }
            }
            
            if (*ready == NULL && *waiting == NULL && *running == NULL) { // If all queues are finished then print final table and end loop
                
                if (*terminated != NULL) {
                    cout << "\nTotal time: " << t << "  CPU Utilization: " << 100*(1-(idleTime/t)) << "%" << endl;
                    (*terminated)->printTable(*terminated, 0);
                }
                return;
            } 
            
            if (*running == NULL) { idleTime++; } // Otherwise, if there is no process running then increment the idle time var
                 
            
            programCounter(*ready, *running, *waiting); // Increment the waiting, cpu, and io times
        }
    }
}

void scheduleMLFQ(PCB** ready, PCB** running, PCB** waiting, PCB** terminated) {
    if (ready != NULL && running != NULL && waiting != NULL && terminated != NULL) {
        int idleTime = 0;
        PCB* queue1 = NULL;
        int tq1 = 5;
        PCB* queue2 = NULL;
        int tq2 = 10;
        PCB* queue3 = NULL;
        queue1 = *ready; // Processes are initialized in the ready queue - transfer them to queue1 of MLFQ
        for (int t = 0; t <= 1024; t++) {  // TIME loop - time units t

            if (*waiting != NULL) {
                (*waiting)->waitManage(waiting, ready);
            }
            if (*running != NULL) {
                (*running)->cpuManage(running, waiting, terminated, t);
            }
            if (*running == NULL) {     // Check if no processes are running (no preemption)
                if (*ready != NULL) {  // If there is a process in ready queue:

                    (*ready)->runProcess(ready, running, t);  // Run first process in ready queue
                    (*running)->incrementPC();
                    cout << "\nCurrent Execution time: " << t << endl;
                    (*running)->printStatus();
                    if (*ready != NULL) { cout << "Ready:" << endl; (*ready)->printStatus(); }
                    if (*waiting != NULL) { cout << "Waiting:" << endl; (*waiting)->printStatus(); }
                    if (*terminated != NULL) { cout << "Terminated:" << endl; (*terminated)->printStatus(); }

                    
                }
            }

            if (*ready == NULL && *waiting == NULL && *running == NULL) { // If all queues are finished then print final table and end loop

                if (*terminated != NULL) {
                    cout << "\nTotal time: " << t << "  CPU Utilization: " << 100 * (1 - (idleTime / t)) << "%" << endl;
                    (*terminated)->printTable(*terminated, 0);
                }
                return;
            }
            else {
                if (*running == NULL) { idleTime++; } // Otherwise, if there is no process running then increment the idle time var
                programCounter(*ready, *running, *waiting); // Increment the waiting, cpu, and io times
            }

        }
    }
}

void simulation(string scheduler) {
    // Create the states of OS
    PCB* readyList = nullptr; // first PCB of the ready list
    PCB* runningList = nullptr; // PCB that is currently running in the CPU
    PCB* waitingList = nullptr; // PCB waiting list
    PCB* terminatedList = nullptr; // terminated processes

    // Create the process data inputs for process CPU bursts and I/O times
    int P1[] = { 5, 27, 3, 31, 5, 43, 4, 18, 6, 22, 4, 26, 3, 24, 5 };
    int P2[] = { 4, 48, 5, 44, 7, 42, 12, 37, 9, 76, 4, 41, 9, 31, 7, 43, 8 };
    int P3[] = { 8, 33, 12, 41, 18, 65, 14, 21, 4, 61, 15, 18, 14, 26, 5, 31, 6 };
    int P4[] = { 3, 35, 4, 41, 5, 45, 3, 51, 4, 61, 5, 54, 6, 82, 5, 77, 3 };
    int P5[] = { 16, 24, 17, 21, 5, 36, 16, 26, 7, 31, 13, 28, 11, 21, 6, 13, 3, 11, 4 };
    int P6[] = { 11, 22, 4, 8, 5, 10, 6, 12, 7, 14, 9, 18, 12, 24, 15, 30, 8 };
    int P7[] = { 14, 46, 17, 41, 11, 42, 15, 21, 4, 32, 7, 19, 16, 33, 10 };
    int P8[] = { 4, 14, 5, 33, 6, 51, 14, 73, 16, 87, 6 };

    // Create array of PCBs from process data
    PCB pcbArray[] = {
        PCB(1,P1,sizeof(P1) / sizeof(P1[0])),
        PCB(2,P2,sizeof(P2) / sizeof(P2[0])),
        PCB(3,P3,sizeof(P3) / sizeof(P3[0])),
        PCB(4,P4,sizeof(P4) / sizeof(P4[0])),
        PCB(5,P5,sizeof(P5) / sizeof(P5[0])),
        PCB(6,P6,sizeof(P6) / sizeof(P6[0])),
        PCB(7,P7,sizeof(P7) / sizeof(P7[0])),
        PCB(8,P8,sizeof(P8) / sizeof(P8[0]))
    };
    for (int i = 0; i < sizeof(pcbArray)/sizeof(pcbArray[0]); i++) {
        (pcbArray[i]).admitProcess(&readyList);
    } // put processes into ready list

    if (scheduler == "SJF") {
        scheduleSJF(&readyList, &runningList, &waitingList, &terminatedList);
    }
    else if (scheduler == "FCFS") {
        scheduleFCFS(&readyList, &runningList, &waitingList, &terminatedList);
    }
    else if (scheduler == "MLFQ") {
        scheduleMLFQ(&readyList, &runningList, &waitingList, &terminatedList);
    }
    else { cout << "\nInvalid scheduling type! (check main)\nValid types are:\nSJF\nFCFS\nMLFQ\n"; }

    return;
}




int main()
{
    //simulation(string("SJF"));
    simulation(string("FCFS"));
    //simulation(string("MLFQ"));
    return 0;
}
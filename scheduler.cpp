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
    PCB* getNext(int previousProcessID); // Used to go round robin through all processes
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

PCB* PCB::getLast() {
    if (pcbPointer == NULL) { return this; }
    else { return pcbPointer->getLast(); }
}

PCB* PCB::getShortest() {
    if (pcbPointer == NULL) { return this; }
    else {
        if (pcbPointer->getShortest()->processData[pcbPointer->getShortest()->dataIndex + 1] < processData[dataIndex + 1]) { return pcbPointer->getShortest(); }
        else { return this; }
    }
}

PCB* PCB::getNext(int previousProcessID) {

}

void PCB::admitProcess(PCB** ready) {
    if ((processState == "new") && ready != NULL) {
        processState = "ready";
        if (*ready == NULL) { *ready = this; }
        else { (*ready)->getLast()->pcbPointer = this; }
    }
    else { cout << "Could not admit process " << processID << " - [" << processState << "]" << endl; }
}

void PCB::runProcess(PCB** ready, PCB** running, int time) { // moves process from ready list to running, preempting 
    if (processState == "ready") {
        processState = "running";
        if (*ready == this) {
            *ready = pcbPointer;
        }
        else {
            PCB* temp = *ready;
            if (temp->pcbPointer != NULL) {
                while (temp->pcbPointer != this) { temp = temp->pcbPointer; }
                temp->pcbPointer = temp->pcbPointer->pcbPointer;
            }
        }
        
        *running = this;
        pcbPointer = NULL;
        if (responseTime == -1) { responseTime = time; } // set response time when process is ran (if first time -1)
    }
    else { cout << "Could not run process " << processID << " - [" << processState << "]" << endl; }
}

void PCB::waitProcess(PCB** running, PCB** waiting) { // moves process from running to waiting list
    if (processState == "running") {
        processState = "waiting";
        if (*waiting == NULL) { *waiting = this; }
        else { (*waiting)->getLast()->pcbPointer = this; }
        *running = NULL;
        pcbPointer = NULL;
    }
    else { cout << "Could not move process " << processID << " to waiting queue - " << "[" << processState << "]" << endl; }
}

void PCB::readyProcess(PCB** waiting, PCB** ready) {
    if (processState == "waiting") {
        processState = "ready";
        if (*waiting == this) {
            *waiting = pcbPointer;
        }
        else {
            PCB* temp = *waiting;
            if (temp->pcbPointer != NULL) {
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

void PCB::incrementPC() {
    if (dataIndex + 1 < dataSize) {
        dataIndex++;
        dataTime += processData[dataIndex];
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
        if (search != NULL) { cout << "P" << search->processID << "    Tw = " << (search->waitingTime) << "    Ttr = " << (search->waitingTime + search->ioTime + search->cpuTime) << "    Tr = " << search->responseTime << endl; }
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



// Non-member functions: \\

void programCounter(PCB* ready, PCB* running, PCB* waiting) {
    if (ready != NULL) { ready->waitTiming(); }
    if (running != NULL) { running->cpuTiming(); }
    if (waiting != NULL) { waiting->ioTiming(); }
}

void scheduleSJF(PCB** ready, PCB** running, PCB** waiting, PCB** terminated) {
    if (ready != NULL && running != NULL && waiting != NULL && terminated != NULL) {
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
            else {
                if (*running == NULL) { idleTime++; } // Otherwise, if there is no process running then increment the idle time var
                 // Increment the waiting, cpu, and io times
            }
            programCounter(*ready, *running, *waiting);
        }
    }
}

//********
void scheduleMLFQ(PCB** ready, PCB** running, PCB** waiting, PCB** terminated) {
    if (ready != NULL && running != NULL && waiting != NULL && terminated != NULL) {
        int idleTime = 0;
        int prevProcID = 0;
        PCB* queue1 = NULL;
        PCB* queue2 = NULL;
        PCB* queue3 = NULL;
        for (int t = 0; t <= 1024; t++) {  // TIME loop - time units t

            if (*waiting != NULL) {
                (*waiting)->waitManage(waiting, ready);
            }
            if (*running != NULL) {
                (*running)->cpuManage(running, waiting, terminated, t);
            }
            if (*running == NULL) {     // Check if no processes are running (no preemption)
                if (*ready != NULL) {  // If there is a process in ready queue:

                    (*ready)->getNext(prevProcID)->runProcess(ready, running, t);  // Run first process in ready queue
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
#include <iostream>
#include <string>
using namespace std;

class PCB;
void simulation(string scheduler); // Prototype for simulation function to run the sim

class PCB
{
    public:
        PCB * getTail();
        PCB * pcbPointer;        // Pointer to next PCB
        PCB(){ initialize(); } // Use initialize() to create the new process PCB
        PCB(int num, int array[], PCB ** readyPointer); // Explicit value constructor assigns process ID and go to ready state
        
    //private:
        void initialize(); //Called by constructor to initialize PCB in READY state

        string processState;  // Keep track of which state this process should be in
        int processID;       // Keep track of process number (P1-8)
      
        // Program Counter: // [The following int are part of the program counter]
        int* dataPointer;  // Pointer to the first CPU burst of the given data array
        int waitingTime;  // Accumulate 1 for every time unit spent in waiting queue
        int cpuTime;     // Accumulate 1 for every time unit spent in CPU
        int ioTime;     // Accumulate 1 for every time unit spent in I/O
};

PCB * PCB::getTail(){
    if(this->pcbPointer==NULL){ return this;}
    else{ return this->pcbPointer->getTail();}
}

void PCB::initialize(){
    processState = "new"; // Start process PCB in new state
    waitingTime = 0;     // Accumulate 1 for every time unit spent in waiting queue
    cpuTime = 0;        // Accumulate 1 for every time unit spent in CPU
    ioTime = 0;        // Accumulate 1 for every time unit spent in I/O
}

PCB::PCB(int num, int array[], PCB ** readyListPointer){
    initialize();            // Initialize process at new state, and program counter set 0
    processState = "ready"; // After NEW process, it goes into ready state first
    processID = num;       // Initialize process with given process ID (P1-8)
    dataPointer = array;  // Assign the input array to the data pointer
    if(readyListPointer != NULL && *readyListPointer != NULL){ (*readyListPointer)->getTail()->pcbPointer = this;}
    else{*readyListPointer = this;}
}

void scheduleSJF(PCB ** ready, PCB ** running, PCB ** waiting, PCB ** terminated){
    for(unsigned int t = 0; t<=1024; t++){// TIME
        if(*running == NULL){
            *running = *ready;
            (*running)-> processState = "running";
            *ready = (*ready)->pcbPointer;
        }
        cout<< (*ready)->processID << endl;
        // c - make member function for pcb that checks if running and if so then if the running process has finished burst by looking at cpu and io time and THEN if running is empty, run the next process in ready queue
    }
}

void scheduleFCFS(PCB *ready, PCB *running, PCB *waiting, PCB *terminated){
    for(unsigned int t = 0; t<=1024; t++){// TIME
        
    }
}

void scheduleMLFQ(PCB *ready, PCB *running, PCB *waiting, PCB *terminated){
    for(unsigned int t = 0; t<=1024; t++){// TIME
        //
    }
}

void simulation(string scheduler){
    // Create the states of OS
    PCB *readyList; // first PCB of the ready list
    PCB *runningList; // PCB that is currently running in the CPU
    PCB *waitingList; // PCB waiting list
    PCB *terminatedList; // terminated processes

    // Create the data inputs for process CPU bursts and I/O times
    int P1[] = {5, 27, 3, 31, 5, 43, 4, 18, 6, 22, 4, 26, 3, 24, 5};
    int P2[] = {4, 48, 5, 44, 7, 42, 12, 37, 9, 76, 4, 41, 9, 31, 7, 43, 8};
    int P3[] = {8, 33, 12, 41, 18, 65, 14, 21, 4, 61, 15, 18, 14, 26, 5, 31, 6};
    int P4[] = {3, 35, 4, 41, 5, 45, 3, 51, 4, 61, 5, 54, 6, 82, 5, 77, 3};
    int P5[] = {16, 24, 17, 21, 5, 36, 16, 26, 7, 31, 13, 28, 11, 21, 6, 13, 3, 11, 4};
    int P6[] = {11, 22, 4, 8, 5, 10, 6, 12, 7, 14, 9, 18, 12, 24, 15, 30, 8};
    int P7[] = {14, 46, 17, 41, 11, 42, 15, 21, 4, 32, 7, 19, 16, 33, 10};
    int P8[] = {4, 14, 5, 33, 6, 51, 14, 73, 16, 87, 6};

    // Create array of PCBs 
    PCB pcbArray[] = {PCB(1,P1,&readyList), PCB(2,P2,&readyList), PCB(3,P3,&readyList) ,PCB(4,P4,&readyList), PCB(5,P5,&readyList), PCB(6,P6,&readyList), PCB(7,P7,&readyList), PCB(8,P8,&readyList)};

    if(scheduler=="SJF"){
        scheduleSJF(&readyList, &runningList, &waitingList, &terminatedList);
    } else if (scheduler=="FCFS"){
        scheduleFCFS(readyList, runningList, waitingList, terminatedList);
    } else if(scheduler=="MLFQ"){
        scheduleMLFQ(readyList, runningList, waitingList, terminatedList);
    } else { cout << "\nInvalid scheduling type! (check main)\nValid types are:\nSJF\nFCFS\nMLFQ\n";}
    
}




int main()
{
    simulation(string("SJF" ));
    simulation(string("FCFS"));
    simulation(string("MLFQ"));
    return 0;
}
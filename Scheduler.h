// Scheduler.h
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <fstream>
#include <climits>
#include <map>
#include <numeric>

using namespace std;

// Declaration of the Process struct
struct Process {
    int id;
    int arrival_time;
    vector<int> cpu_bursts;
    vector<int> io_bursts;
    int current_burst;
    int remaining_time;
    int turnaround_time;
    int waiting_time;
    int completion_time;

    Process(int id, int arrival_time, vector<int> cpu_bursts, vector<int> io_bursts);
};

// Function prototypes
vector<Process> parseWorkloadFile(const string &file_path);
void runFIFO(vector<Process> &processes);
void runNonPreemptiveSJF(vector<Process> &processes);
void runPreemptiveSJF(vector<Process> &processes);
void runCFS(vector<Process> &processes);

#endif // SCHEDULER_H

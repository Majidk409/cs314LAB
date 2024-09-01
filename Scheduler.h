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
#include <sstream>
#include <chrono>

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

// Definition of the Process constructor
Process::Process(int id, int arrival_time, vector<int> cpu_bursts, vector<int> io_bursts)
    : id(id), arrival_time(arrival_time), cpu_bursts(cpu_bursts), io_bursts(io_bursts),
      current_burst(0), remaining_time(cpu_bursts.empty() ? 0 : cpu_bursts[0]),
      turnaround_time(0), waiting_time(0), completion_time(0) {}

// Function to parse the workload file and create a list of processes
vector<Process> parseWorkloadFile(const string &file_path) {
    ifstream infile(file_path);
    vector<Process> processes;  
    int arrival_time, burst;
    int process_id = 0;
    string line;

    // Outer loop: Reads each line for a new process
    while (getline(infile, line)) {
        istringstream iss(line);
        iss >> arrival_time;  // Read arrival time for a new process

        vector<int> cpu_bursts;
        vector<int> io_bursts;

        // Inner loop: Reads bursts until -1 is found
        while (iss >> burst && burst != -1) {
            cpu_bursts.push_back(burst);  // Add CPU burst

            if (iss >> burst && burst != -1) {  // Read IO burst if available
                io_bursts.push_back(burst);
            }
        }

        // Add the parsed process to the list
        processes.emplace_back(process_id++, arrival_time, cpu_bursts, io_bursts);
    }

    return processes;
}


#endif // SCHEDULER_H

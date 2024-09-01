#include "Scheduler.h"
#include <sstream>
#include <chrono>

// Definition of the Process constructor
Process::Process(int id, int arrival_time, vector<int> cpu_bursts, vector<int> io_bursts)
    : id(id), arrival_time(arrival_time), cpu_bursts(cpu_bursts), io_bursts(io_bursts),
      current_burst(0), remaining_time(cpu_bursts.empty() ? 0 : cpu_bursts[0]),
      turnaround_time(0), waiting_time(0), completion_time(0) {}

// Implementations of functions
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

#include <chrono>
#include <iostream>
#include <vector>

using namespace std;
using namespace std::chrono;

void runFIFO(vector<Process> &processes) {
    int current_time = 0;
    int total_turnaround_time = 0, total_waiting_time = 0;
    int max_turnaround_time = 0, max_waiting_time = 0;

    auto start_time = high_resolution_clock::now();

    for (auto &process : processes) {
        if (current_time < process.arrival_time) {
            current_time = process.arrival_time;
        }
        process.waiting_time = current_time - process.arrival_time;
        for (int burst : process.cpu_bursts) {
            current_time += burst;
        }
        process.turnaround_time = current_time - process.arrival_time;
        process.completion_time = current_time;

        total_turnaround_time += process.turnaround_time;
        total_waiting_time += process.waiting_time;

        if (process.turnaround_time > max_turnaround_time) max_turnaround_time = process.turnaround_time;
        if (process.waiting_time > max_waiting_time) max_waiting_time = process.waiting_time;
    }

    auto end_time = high_resolution_clock::now();
    duration<double> elapsed_time = end_time - start_time;

    int n = processes.size();
    cout << "FIFO Average Turnaround Time: " << (double)total_turnaround_time / n << endl;
    cout << "FIFO Max Turnaround Time: " << max_turnaround_time << endl;
    cout << "FIFO Average Waiting Time: " << (double)total_waiting_time / n << endl;
    cout << "FIFO Max Waiting Time: " << max_waiting_time << endl;
    cout << "FIFO Throughput: " << (double)n / elapsed_time.count() << " processes per second" << endl;
    cout << "FIFO Simulator Run Time: " << elapsed_time.count() << " seconds" << endl;
}


void runNonPreemptiveSJF(vector<Process> &processes) {
    int current_time = 0;
    int total_turnaround_time = 0, total_waiting_time = 0;
    int max_turnaround_time = 0, max_waiting_time = 0;
    vector<Process*> ready_queue;

    auto start_time = chrono::steady_clock::now();

    while (!processes.empty() || !ready_queue.empty()) {
        for (auto it = processes.begin(); it != processes.end();) {
            if (it->arrival_time <= current_time) {
                ready_queue.push_back(&(*it));
                it = processes.erase(it);
            } else {
                ++it;
            }
        }

        if (ready_queue.empty()) {
            current_time++;
            continue;
        }

        auto shortest_process = min_element(ready_queue.begin(), ready_queue.end(),
                                            [](Process *a, Process *b) { return a->remaining_time < b->remaining_time; });

        Process *process = *shortest_process;
        ready_queue.erase(shortest_process);

        process->waiting_time = current_time - process->arrival_time;
        current_time += process->remaining_time;
        process->turnaround_time = current_time - process->arrival_time;
        process->completion_time = current_time;

        total_turnaround_time += process->turnaround_time;
        total_waiting_time += process->waiting_time;

        if (process->turnaround_time > max_turnaround_time) max_turnaround_time = process->turnaround_time;
        if (process->waiting_time > max_waiting_time) max_waiting_time = process->waiting_time;
    }

    auto end_time = chrono::steady_clock::now();
    chrono::duration<double> elapsed_time = end_time - start_time;

    int n = processes.size();
    cout << "Non-Preemptive SJF Average Turnaround Time: " << (double)total_turnaround_time / n << endl;
    cout << "Non-Preemptive SJF Max Turnaround Time: " << max_turnaround_time << endl;
    cout << "Non-Preemptive SJF Average Waiting Time: " << (double)total_waiting_time / n << endl;
    cout << "Non-Preemptive SJF Max Waiting Time: " << max_waiting_time << endl;
    cout << "Non-Preemptive SJF Throughput: " << (double)n / elapsed_time.count() << " processes per second" << endl;
    cout << "Non-Preemptive SJF Simulator Run Time: " << elapsed_time.count() << " seconds" << endl;
}

void runPreemptiveSJF(vector<Process> &processes) {
    int current_time = 0;
    int total_turnaround_time = 0, total_waiting_time = 0;
    int max_turnaround_time = 0, max_waiting_time = 0;
    vector<Process*> ready_queue;

    auto start_time = chrono::steady_clock::now();

    while (!processes.empty() || !ready_queue.empty()) {
        for (auto it = processes.begin(); it != processes.end();) {
            if (it->arrival_time <= current_time) {
                ready_queue.push_back(&(*it));
                it = processes.erase(it);
            } else {
                ++it;
            }
        }

        if (ready_queue.empty()) {
            current_time++;
            continue;
        }

        auto shortest_process = min_element(ready_queue.begin(), ready_queue.end(),
                                            [](Process *a, Process *b) { return a->remaining_time < b->remaining_time; });

        Process *process = *shortest_process;
        process->remaining_time--;
        current_time++;

        if (process->remaining_time == 0) {
            ready_queue.erase(shortest_process);
            process->turnaround_time = current_time - process->arrival_time;
            process->completion_time = current_time;
            total_turnaround_time += process->turnaround_time;
            total_waiting_time += (process->turnaround_time - accumulate(process->cpu_bursts.begin(), process->cpu_bursts.end(), 0));
            
            if (process->turnaround_time > max_turnaround_time) max_turnaround_time = process->turnaround_time;
            if ((process->turnaround_time - accumulate(process->cpu_bursts.begin(), process->cpu_bursts.end(), 0)) > max_waiting_time) 
                max_waiting_time = (process->turnaround_time - accumulate(process->cpu_bursts.begin(), process->cpu_bursts.end(), 0));
        }
    }

    auto end_time = chrono::steady_clock::now();
    chrono::duration<double> elapsed_time = end_time - start_time;

    int n = processes.size();
    cout << "Preemptive SJF Average Turnaround Time: " << (double)total_turnaround_time / n << endl;
    cout << "Preemptive SJF Max Turnaround Time: " << max_turnaround_time << endl;
    cout << "Preemptive SJF Average Waiting Time: " << (double)total_waiting_time / n << endl;
    cout << "Preemptive SJF Max Waiting Time: " << max_waiting_time << endl;
    cout << "Preemptive SJF Throughput: " << (double)n / elapsed_time.count() << " processes per second" << endl;
    cout << "Preemptive SJF Simulator Run Time: " << elapsed_time.count() << " seconds" << endl;
}

void runCFS(vector<Process> &processes) {
    int current_time = 0;
    int total_turnaround_time = 0, total_waiting_time = 0;
    int max_turnaround_time = 0, max_waiting_time = 0;
    map<int, Process*> cfs_tree;

    auto start_time = chrono::steady_clock::now();

    while (!processes.empty() || !cfs_tree.empty()) {
        for (auto it = processes.begin(); it != processes.end();) {
            if (it->arrival_time <= current_time) {
                cfs_tree[it->remaining_time] = &(*it);
                it = processes.erase(it);
            } else {
                ++it;
            }
        }

        if (cfs_tree.empty()) {
            current_time++;
            continue;
        }

        auto first_process = cfs_tree.begin();
        Process *process = first_process->second;
        process->remaining_time--;
        current_time++;

        if (process->remaining_time == 0) {
            cfs_tree.erase(first_process);
            process->turnaround_time = current_time - process->arrival_time;
            process->completion_time = current_time;
            total_turnaround_time += process->turnaround_time;
            total_waiting_time += (process->turnaround_time - accumulate(process->cpu_bursts.begin(), process->cpu_bursts.end(), 0));
            
            if (process->turnaround_time > max_turnaround_time) max_turnaround_time = process->turnaround_time;
            if ((process->turnaround_time - accumulate(process->cpu_bursts.begin(), process->cpu_bursts.end(), 0)) > max_waiting_time) 
                max_waiting_time = (process->turnaround_time - accumulate(process->cpu_bursts.begin(), process->cpu_bursts.end(), 0));
        }
    }

    auto end_time = chrono::steady_clock::now();
    chrono::duration<double> elapsed_time = end_time - start_time;

    int n = processes.size();
    cout << "CFS Average Turnaround Time: " << (double)total_turnaround_time / n << endl;
    cout << "CFS Max Turnaround Time: " << max_turnaround_time << endl;
    cout << "CFS Average Waiting Time: " << (double)total_waiting_time / n << endl;
    cout << "CFS Max Waiting Time: " << max_waiting_time << endl;
    cout << "CFS Throughput: " << (double)n / elapsed_time.count() << " processes per second" << endl;
    cout << "CFS Simulator Run Time: " << elapsed_time.count() << " seconds" << endl;
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <scheduling-algorithm> <path-to-workload-description-file>\n";
        return 1;
    }

    string algorithm = argv[1];
    string file_path = argv[2];

    vector<Process> processes = parseWorkloadFile(file_path);

    if (algorithm == "FIFO") {
        runFIFO(processes);
    } else if (algorithm == "SJF") {
        runNonPreemptiveSJF(processes);
    } else if (algorithm == "SJF-Preemptive") {
        runPreemptiveSJF(processes);
    } else if (algorithm == "CFS") {
        runCFS(processes);
    } else {
        cerr << "Unknown scheduling algorithm: " << algorithm << "\n";
        return 1;
    }

    return 0;
}

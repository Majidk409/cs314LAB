// Scheduler.cpp
#include "Scheduler.h"

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
    while (infile >> arrival_time) {
        vector<int> cpu_bursts;
        vector<int> io_bursts;
        while (infile >> burst && burst != -1) {
            cpu_bursts.push_back(burst);
            if (infile.peek() != '\n') {
                infile >> burst;
                io_bursts.push_back(burst);
            }
        }
        processes.emplace_back(process_id++, arrival_time, cpu_bursts, io_bursts);
    }
    return processes;
}

void runFIFO(vector<Process> &processes) {
    int current_time = 0;
    int total_turnaround_time = 0, total_waiting_time = 0;

    for (auto &process : processes) {
        if (current_time < process.arrival_time) {
            current_time = process.arrival_time; // Process waits until it arrives
        }
        process.waiting_time = current_time - process.arrival_time; // Time spent waiting in the queue
        for (int burst : process.cpu_bursts) {
            current_time += burst; // Adding CPU bursts to current time
        }
        process.turnaround_time = current_time - process.arrival_time; // Total time from arrival to completion
        process.completion_time = current_time; // When the process completes

        total_turnaround_time += process.turnaround_time;
        total_waiting_time += process.waiting_time;
    }

    int n = processes.size();
    cout << "FIFO Average Turnaround Time: " << (double)total_turnaround_time / n << endl;
    cout << "FIFO Average Waiting Time: " << (double)total_waiting_time / n << endl;
}

void runNonPreemptiveSJF(vector<Process> &processes) {
    int current_time = 0;
    int total_turnaround_time = 0, total_waiting_time = 0;
    vector<Process*> ready_queue;

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
    }

    int n = processes.size();
    cout << "Non-Preemptive SJF Average Turnaround Time: " << (double)total_turnaround_time / n << endl;
    cout << "Non-Preemptive SJF Average Waiting Time: " << (double)total_waiting_time / n << endl;
}

void runPreemptiveSJF(vector<Process> &processes) {
    int current_time = 0;
    int total_turnaround_time = 0, total_waiting_time = 0;
    vector<Process*> ready_queue;

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
        }
    }

    int n = processes.size();
    cout << "Preemptive SJF Average Turnaround Time: " << (double)total_turnaround_time / n << endl;
    cout << "Preemptive SJF Average Waiting Time: " << (double)total_waiting_time / n << endl;
}

void runCFS(vector<Process> &processes) {
    int current_time = 0;
    int total_turnaround_time = 0, total_waiting_time = 0;
    map<int, Process*> cfs_tree;

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
        }
    }

    int n = processes.size();
    cout << "CFS Average Turnaround Time: " << (double)total_turnaround_time / n << endl;
    cout << "CFS Average Waiting Time: " << (double)total_waiting_time / n << endl;
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

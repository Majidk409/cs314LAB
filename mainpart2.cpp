#include "Scheduler.h"
#include <sstream>
#include <chrono>
#include <queue>
#include <map>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <fstream>
#include <vector>

using namespace std::chrono;
using namespace std;

// Definition of the Processor struct
struct Processor {
    int id;                     // Processor ID (0 or 1)
    int current_time;           // Current time on the processor
    queue<Process*> ready_queue; // Queue of processes ready to run on this processor
};

// Function to run the FIFO scheduling algorithm with separate metrics for each processor
void runFIFO(vector<Processor> &processors) {
    auto start_time = high_resolution_clock::now();

    int processor_index = 0;  // To keep track of which processor we're working on
    for (auto &processor : processors) {
        int total_turnaround_time = 0, total_waiting_time = 0;
        int max_turnaround_time = 0, max_waiting_time = 0;
        int total_processes = 0;

        while (!processor.ready_queue.empty()) {
            Process *process = processor.ready_queue.front();
            processor.ready_queue.pop();

            if (processor.current_time < process->arrival_time) {
                processor.current_time = process->arrival_time;
            }

            process->waiting_time = processor.current_time - process->arrival_time;
            for (int burst : process->cpu_bursts) {
                processor.current_time += burst;
            }
            process->turnaround_time = processor.current_time - process->arrival_time;
            process->completion_time = processor.current_time;

            total_turnaround_time += process->turnaround_time;
            total_waiting_time += process->waiting_time;
            total_processes++;

            if (process->turnaround_time > max_turnaround_time) {
                max_turnaround_time = process->turnaround_time;
            }
            if (process->waiting_time > max_waiting_time) {
                max_waiting_time = process->waiting_time;
            }
        }

        // Compute the metrics for the current processor
        int n = total_processes;
        cout << "Processor " << processor_index << " - FIFO Metrics:" << endl;
        cout << "Average Turnaround Time: " << (double)total_turnaround_time / n << endl;
        cout << "Max Turnaround Time: " << max_turnaround_time << endl;
        cout << "Average Waiting Time: " << (double)total_waiting_time / n << endl;
        cout << "Max Waiting Time: " << max_waiting_time << endl;
        cout << "Throughput: " << (double)n / (processor.current_time / 1e6) << " processes per second" << endl; // Processor throughput based on its current time
        cout << "----------------------------------" << endl;

        processor_index++;  // Move to the next processor index
    }

    auto end_time = high_resolution_clock::now();
    duration<double> elapsed_time = end_time - start_time;

    // Overall simulation time
    cout << "Overall FIFO Simulator Run Time: " << elapsed_time.count() << " seconds" << endl;
}

void runNonPreemptiveSJF(vector<Processor> &processors) {
    int total_turnaround_time = 0, total_waiting_time = 0;
    int max_turnaround_time = 0, max_waiting_time = 0;
    int total_processes = 0;

    auto start_time = chrono::steady_clock::now();

    for (auto &processor : processors) {
        auto &ready_queue = processor.ready_queue;
        int &current_time = processor.current_time;

        while (!ready_queue.empty()) {
            vector<Process *> process_vector;

            while (!ready_queue.empty()) {
                process_vector.push_back(ready_queue.front());
                ready_queue.pop();
            }

            sort(process_vector.begin(), process_vector.end(), [](Process *a, Process *b) {
                return a->remaining_time < b->remaining_time;
            });

            for (auto process : process_vector) {
                process->waiting_time = current_time - process->arrival_time;
                current_time += process->remaining_time;
                process->turnaround_time = current_time - process->arrival_time;
                process->completion_time = current_time;

                total_turnaround_time += process->turnaround_time;
                total_waiting_time += process->waiting_time;

                if (process->turnaround_time > max_turnaround_time) max_turnaround_time = process->turnaround_time;
                if (process->waiting_time > max_waiting_time) max_waiting_time = process->waiting_time;

                total_processes++;
            }
        }
    }

    auto end_time = chrono::steady_clock::now();
    chrono::duration<double> elapsed_time = end_time - start_time;

    cout << "Non-Preemptive SJF Average Turnaround Time: " << (double)total_turnaround_time / total_processes << endl;
    cout << "Non-Preemptive SJF Max Turnaround Time: " << max_turnaround_time << endl;
    cout << "Non-Preemptive SJF Average Waiting Time: " << (double)total_waiting_time / total_processes << endl;
    cout << "Non-Preemptive SJF Max Waiting Time: " << max_waiting_time << endl;
    cout << "Non-Preemptive SJF Throughput: " << (double)total_processes / elapsed_time.count() << " processes per second" << endl;
    cout << "Non-Preemptive SJF Simulator Run Time: " << elapsed_time.count() << " seconds" << endl;
}

void runPreemptiveSJF(vector<Processor> &processors) {
    int total_turnaround_time = 0, total_waiting_time = 0;
    int max_turnaround_time = 0, max_waiting_time = 0;
    int total_processes = 0;

    auto start_time = chrono::steady_clock::now();

    for (auto &processor : processors) {
        auto &ready_queue = processor.ready_queue;
        int &current_time = processor.current_time;
        vector<Process *> process_vector;

        while (!ready_queue.empty() || !process_vector.empty()) {
            while (!ready_queue.empty()) {
                process_vector.push_back(ready_queue.front());
                ready_queue.pop();
            }

            if (process_vector.empty()) {
                current_time++;
                continue;
            }

            auto shortest_process = min_element(process_vector.begin(), process_vector.end(),
                                                [](Process *a, Process *b) { return a->remaining_time < b->remaining_time; });

            Process *process = *shortest_process;
            process->remaining_time--;
            current_time++;

            if (process->remaining_time == 0) {
                process_vector.erase(shortest_process);
                process->turnaround_time = current_time - process->arrival_time;
                process->completion_time = current_time;
                total_turnaround_time += process->turnaround_time;
                total_waiting_time += (process->turnaround_time - accumulate(process->cpu_bursts.begin(), process->cpu_bursts.end(), 0));

                if (process->turnaround_time > max_turnaround_time) max_turnaround_time = process->turnaround_time;
                if ((process->turnaround_time - accumulate(process->cpu_bursts.begin(), process->cpu_bursts.end(), 0)) > max_waiting_time)
                    max_waiting_time = (process->turnaround_time - accumulate(process->cpu_bursts.begin(), process->cpu_bursts.end(), 0));

                total_processes++;
            }
        }
    }

    auto end_time = chrono::steady_clock::now();
    chrono::duration<double> elapsed_time = end_time - start_time;

    cout << "Preemptive SJF Average Turnaround Time: " << (double)total_turnaround_time / total_processes << endl;
    cout << "Preemptive SJF Max Turnaround Time: " << max_turnaround_time << endl;
    cout << "Preemptive SJF Average Waiting Time: " << (double)total_waiting_time / total_processes << endl;
    cout << "Preemptive SJF Max Waiting Time: " << max_waiting_time << endl;
    cout << "Preemptive SJF Throughput: " << (double)total_processes / elapsed_time.count() << " processes per second" << endl;
    cout << "Preemptive SJF Simulator Run Time: " << elapsed_time.count() << " seconds" << endl;
}

void runCFS(vector<Processor> &processors) {
    int total_turnaround_time = 0, total_waiting_time = 0;
    int max_turnaround_time = 0, max_waiting_time = 0;
    int total_processes = 0;

    auto start_time = chrono::steady_clock::now();

    for (auto &processor : processors) {
        auto &ready_queue = processor.ready_queue;
        int &current_time = processor.current_time;
        map<int, Process*> cfs_tree;

        while (!ready_queue.empty() || !cfs_tree.empty()) {
            while (!ready_queue.empty()) {
                Process *process = ready_queue.front();
                cfs_tree[process->remaining_time] = process;
                ready_queue.pop();
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

                total_processes++;
            }
        }
    }

    auto end_time = chrono::steady_clock::now();
    chrono::duration<double> elapsed_time = end_time - start_time;

    cout << "CFS Average Turnaround Time: " << (double)total_turnaround_time / total_processes << endl;
    cout << "CFS Max Turnaround Time: " << max_turnaround_time << endl;
    cout << "CFS Average Waiting Time: " << (double)total_waiting_time / total_processes << endl;
    cout << "CFS Max Waiting Time: " << max_waiting_time << endl;
    cout << "CFS Throughput: " << (double)total_processes / elapsed_time.count() << " processes per second" << endl;
    cout << "CFS Simulator Run Time: " << elapsed_time.count() << " seconds" << endl;
}

// Main function
int main(int argc, char *argv[]) {
    if (argc < 3) {
        cerr << "Usage: " << argv[0] << " <scheduling-algorithm> <path-to-workload-description-file>\n";
        return 1;
    }

    string algorithm = argv[1];
    string file_path = argv[2];

    vector<Process> processes = parseWorkloadFile(file_path);

    vector<Processor> processors(2); // Initialize two processors

    // Initialize processors
    for (int i = 0; i < 2; ++i) {
        processors[i].id = i;
        processors[i].current_time = 0;
    }

    // Distribute processes between the two processors
    for (auto &process : processes) {
        int processor_id = process.id % 2;
        processors[processor_id].ready_queue.push(&process);
    }

    if (algorithm == "FIFO") {
        runFIFO(processors);
    } else if (algorithm == "SJF") {
        runNonPreemptiveSJF(processors);
    } else if (algorithm == "SJF-Preemptive") {
        runPreemptiveSJF(processors);
    } else if (algorithm == "CFS") {
        runCFS(processors);
    } else {
        cerr << "Unknown scheduling algorithm: " << algorithm << "\n";
        return 1;
    }

    return 0;
}


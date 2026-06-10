/*
* Andrei Ciceu
* 251355626
* CS3305 Assignment 4
* March 24, 2026
*
* Screenshots show in order: FCFS, SJF, Quantum3, Quantum12
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100

typedef struct {
    int id;
    int arrival_time;
    int burst_time;       // original burst
    int remaining_burst;  // decremented each tick when active
    int wait_time;
    int turnaround_time;
    int finished;
} Process;

int num_processes = 0;
Process processes[MAX_PROCESSES];

int read_input(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Error: cannot open file '%s'\n", filename);
        return 0;
    }
    char line[256];
    int idx = 0;
    while (fgets(line, sizeof(line), f)) {
        // trim newline
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;
        char name[64];
        int burst;
        if (sscanf(line, "%[^,],%d", name, &burst) != 2) continue;
        // arrival time = index (P0 arrives at 0, P1 at 1, ...)
        processes[idx].id = idx;
        processes[idx].arrival_time = idx;
        processes[idx].burst_time = burst;
        processes[idx].remaining_burst = burst;
        processes[idx].wait_time = 0;
        processes[idx].turnaround_time = 0;
        processes[idx].finished = 0;
        idx++;
    }
    fclose(f);
    num_processes = idx;
    return 1;
}

void print_summary() {
    double total_wait = 0, total_ta = 0;
    for (int i = 0; i < num_processes; i++) {
        printf("\nP%d\n", processes[i].id);
        printf("        Waiting time:   %8d\n", processes[i].wait_time);
        printf("        Turnaround time:%8d\n", processes[i].turnaround_time);
        total_wait += processes[i].wait_time;
        total_ta   += processes[i].turnaround_time;
    }
    printf("\nTotal average waiting time:    %.1f\n", total_wait / num_processes);
    printf("Total average turnaround time: %.1f\n", total_ta   / num_processes);
}

/* ---- FCFS ---- */
void run_fcfs() {
    printf("First Come First Served\n");

    int tick = 0;
    int active = -1;

    while (1) {
        // Check if all done
        int all_done = 1;
        for (int i = 0; i < num_processes; i++)
            if (!processes[i].finished) { all_done = 0; break; }
        if (all_done) break;

        // Select active process (non-preemptive: keep running until done)
        if (active == -1 || processes[active].finished) {
            active = -1;
            // pick first arrived, not finished
            for (int i = 0; i < num_processes; i++) {
                if (!processes[i].finished && processes[i].arrival_time <= tick) {
                    active = i;
                    break;
                }
            }
        }

        // Print tick
        if (active != -1) {
            printf("T%d : P%d - Burst left %d, Wait time %d, Turnaround time %d\n",
                tick,
                processes[active].id,
                processes[active].remaining_burst,
                processes[active].wait_time,
                processes[active].turnaround_time);
        }

        // Update all processes
        for (int i = 0; i < num_processes; i++) {
            if (processes[i].finished) continue;
            if (processes[i].arrival_time > tick) continue;

            if (i == active) {
                // active: decrement burst
                processes[i].remaining_burst--;
                processes[i].turnaround_time++;
                if (processes[i].remaining_burst == 0) {
                    processes[i].finished = 1;
                }
            } else {
                // in ready queue: increment wait and turnaround
                processes[i].wait_time++;
                processes[i].turnaround_time++;
            }
        }

        tick++;
    }

    print_summary();
}

/* ---- SJF (preemptive, based on remaining burst) ---- */
void run_sjf() {
    printf("Shortest Job First\n");

    int tick = 0;
    int active = -1;

    while (1) {
        int all_done = 1;
        for (int i = 0; i < num_processes; i++)
            if (!processes[i].finished) { all_done = 0; break; }
        if (all_done) break;

        // Select: shortest remaining burst among arrived, not finished
        // Ties: current process keeps running (no unnecessary context switch)
        int best = -1;
        for (int i = 0; i < num_processes; i++) {
            if (processes[i].finished) continue;
            if (processes[i].arrival_time > tick) continue;
            if (best == -1 || processes[i].remaining_burst < processes[best].remaining_burst) {
                best = i;
            } else if (processes[i].remaining_burst == processes[best].remaining_burst) {
                // tie: prefer currently active
                if (i == active) best = i;
                // else keep best (lower id / first found)
            }
        }
        // If tie between best and active, keep active
        if (active != -1 && !processes[active].finished &&
            processes[active].arrival_time <= tick &&
            best != -1 &&
            processes[best].remaining_burst == processes[active].remaining_burst) {
            best = active;
        }
        active = best;

        if (active != -1) {
            printf("T%d : P%d - Burst left %d, Wait time %d, Turnaround time %d\n",
                tick,
                processes[active].id,
                processes[active].remaining_burst,
                processes[active].wait_time,
                processes[active].turnaround_time);
        }

        // Update
        for (int i = 0; i < num_processes; i++) {
            if (processes[i].finished) continue;
            if (processes[i].arrival_time > tick) continue;

            if (i == active) {
                processes[i].remaining_burst--;
                processes[i].turnaround_time++;
                if (processes[i].remaining_burst == 0)
                    processes[i].finished = 1;
            } else {
                processes[i].wait_time++;
                processes[i].turnaround_time++;
            }
        }

        tick++;
    }

    print_summary();
}

/* ---- Round Robin ---- */
void run_rr(int quantum) {
    printf("Round Robin with Quantum %d\n", quantum);

    int queue[MAX_PROCESSES * 10000];
    int q_head = 0, q_tail = 0;

    int tick = 0;
    int active = -1;
    int ticks_this_quantum = 0;
    int enqueued[MAX_PROCESSES];
    memset(enqueued, 0, sizeof(enqueued));

    // Enqueue processes arriving at tick 0
    for (int i = 0; i < num_processes; i++) {
        if (processes[i].arrival_time == 0) {
            queue[q_tail++] = i;
            enqueued[i] = 1;
        }
    }

    // Dequeue first active
    if (q_head < q_tail) {
        active = queue[q_head++];
        ticks_this_quantum = 0;
    }

    while (1) {
        int all_done = 1;
        for (int i = 0; i < num_processes; i++)
            if (!processes[i].finished) { all_done = 0; break; }
        if (all_done) break;

        // If no active, try to dequeue
        if (active == -1) {
            // enqueue any newly arrived
            for (int i = 0; i < num_processes; i++) {
                if (!enqueued[i] && !processes[i].finished && processes[i].arrival_time <= tick) {
                    queue[q_tail++] = i;
                    enqueued[i] = 1;
                }
            }
            if (q_head < q_tail) {
                active = queue[q_head++];
                ticks_this_quantum = 0;
            }
        }

        if (active != -1) {
            printf("T%d : P%d - Burst left %d, Wait time %d, Turnaround time %d\n",
                tick,
                processes[active].id,
                processes[active].remaining_burst,
                processes[active].wait_time,
                processes[active].turnaround_time);
        }

        // Update
        for (int i = 0; i < num_processes; i++) {
            if (processes[i].finished) continue;
            if (processes[i].arrival_time > tick) continue;

            if (i == active) {
                processes[i].remaining_burst--;
                processes[i].turnaround_time++;
                if (processes[i].remaining_burst == 0)
                    processes[i].finished = 1;
            } else {
                processes[i].wait_time++;
                processes[i].turnaround_time++;
            }
        }

        ticks_this_quantum++;
        tick++;

        // Context switch if quantum expired or process finished
        int quantum_done = (active != -1) && (ticks_this_quantum >= quantum);
        int proc_done    = (active != -1) && processes[active].finished;

        if (quantum_done || proc_done) {
            // Enqueue processes that arrived at new tick BEFORE re-enqueuing active
            for (int i = 0; i < num_processes; i++) {
                if (!enqueued[i] && !processes[i].finished && processes[i].arrival_time <= tick) {
                    queue[q_tail++] = i;
                    enqueued[i] = 1;
                }
            }
            // Re-enqueue active if not finished
            if (!proc_done) {
                queue[q_tail++] = active;
            }
            active = -1;
            if (q_head < q_tail) {
                active = queue[q_head++];
                ticks_this_quantum = 0;
            }
        } else {
            // mid-quantum: still enqueue newly arrived
            for (int i = 0; i < num_processes; i++) {
                if (!enqueued[i] && !processes[i].finished && processes[i].arrival_time <= tick) {
                    queue[q_tail++] = i;
                    enqueued[i] = 1;
                }
            }
        }
    }

    print_summary();
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s -f|-s|-r [quantum] <filename>\n", argv[0]);
        return 1;
    }

    char *algo = argv[1];
    int quantum = 0;
    char *filename = NULL;

    if (strcmp(algo, "-r") == 0) {
        if (argc < 4) {
            fprintf(stderr, "Error: -r requires a time quantum\n");
            return 1;
        }
        quantum = atoi(argv[2]);
        if (quantum <= 0) {
            fprintf(stderr, "Error: invalid time quantum '%s'\n", argv[2]);
            return 1;
        }
        filename = argv[3];
    } else if (strcmp(algo, "-f") == 0 || strcmp(algo, "-s") == 0) {
        filename = argv[2];
    } else {
        fprintf(stderr, "Error: invalid option '%s'. Use -f, -s, or -r\n", algo);
        return 1;
    }

    if (!read_input(filename)) return 1;
    if (num_processes == 0) {
        fprintf(stderr, "Error: no processes found in input file\n");
        return 1;
    }

    if (strcmp(algo, "-f") == 0) {
        run_fcfs();
    } else if (strcmp(algo, "-s") == 0) {
        run_sjf();
    } else {
        run_rr(quantum);
    }

    return 0;
}
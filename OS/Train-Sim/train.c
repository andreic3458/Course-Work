/*
* Andrei Ciceu 
* 251355626
* CS3305 Assignment 5
* April 7, 2026
*/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define NUM_STATIONS 5
#define TRAIN0_CAP 100
#define TRAIN1_CAP 50

int station_passengers[NUM_STATIONS] = {500, 0, 0, 0, 0};
int station_destination[NUM_STATIONS] = {0, 50, 100, 250, 100};

pthread_mutex_t station_mutex[NUM_STATIONS];
pthread_mutex_t stdout_mutex;

typedef struct {
    int id;
    int capacity;
} Train;

void do_sleep(int passengers) {
    int seconds = passengers / 10;
    if (seconds > 0) sleep(seconds);
}

void *train_thread(void *arg) {
    Train *train = (Train *)arg;
    int id = train->id;
    int cap = train->capacity;
    int on_board = 0;
    int direction = 1; // 1 = forward (0->4), -1 = backward (4->0)
    int current = 0;

    while (1) {
        // Lock station
        pthread_mutex_lock(&station_mutex[current]);
        pthread_mutex_lock(&stdout_mutex);
        printf("Train %d ENTERS Station %d\n", id, current);
        pthread_mutex_unlock(&stdout_mutex);

        if (current == 0) {
            // Pick up passengers
            int to_pick = cap - on_board;
            int available = station_passengers[0];
            if (available == 0 && on_board == 0) {
                pthread_mutex_lock(&stdout_mutex);
                printf("    Station 0 has 0 passengers left to pick up\n");
                printf("    Train %d is at Station 0 and has %d/%d passengers\n", id, on_board, cap);
                printf("        <Nothing more to do>...\n");
                printf("    Train %d is at Station 0 and has %d/%d passengers\n", id, on_board, cap);
                printf("    Station 0 has 0 passengers left to pick up\n");
                pthread_mutex_unlock(&stdout_mutex);

                pthread_mutex_lock(&stdout_mutex);
                printf("Train %d LEAVES Station %d\n", id, current);
                pthread_mutex_unlock(&stdout_mutex);
                pthread_mutex_unlock(&station_mutex[current]);
                break;
            }

            if (available > 0) {
                if (to_pick > available) to_pick = available;

                pthread_mutex_lock(&stdout_mutex);
                printf("    Station 0 has %d passengers left to pick up\n", available);
                printf("    Train %d is at Station 0 and has %d/%d passengers\n", id, on_board, cap);
                printf("        Loading passengers...\n");
                pthread_mutex_unlock(&stdout_mutex);

                do_sleep(to_pick);
                on_board += to_pick;
                station_passengers[0] -= to_pick;

                pthread_mutex_lock(&stdout_mutex);
                printf("    Train %d is at Station 0 and has %d/%d passengers\n", id, on_board, cap);
                printf("    Station 0 has %d passengers left to pick up\n", station_passengers[0]);
                pthread_mutex_unlock(&stdout_mutex);

                direction = 1;
            } else {
                // No passengers to pick up but has on_board to drop off (shouldn't happen at 0, but handle gracefully)
                pthread_mutex_lock(&stdout_mutex);
                printf("    Station 0 has 0 passengers left to pick up\n");
                printf("    Train %d is at Station 0 and has %d/%d passengers\n", id, on_board, cap);
                printf("        <Nothing more to do>...\n");
                printf("    Train %d is at Station 0 and has %d/%d passengers\n", id, on_board, cap);
                printf("    Station 0 has 0 passengers left to pick up\n");
                pthread_mutex_unlock(&stdout_mutex);

                if (on_board == 0) {
                    pthread_mutex_lock(&stdout_mutex);
                    printf("Train %d LEAVES Station %d\n", id, current);
                    pthread_mutex_unlock(&stdout_mutex);
                    pthread_mutex_unlock(&station_mutex[current]);
                    break;
                }
                direction = 1;
            }
        } else {
            // Drop off passengers destined for this station
            int to_drop = on_board < station_destination[current] ? on_board : station_destination[current];
            // Actually: drop off min(on_board, remaining_needed)
            int needed = station_destination[current];
            if (needed <= 0) {
                // Nothing to drop off
                pthread_mutex_lock(&stdout_mutex);
                printf("    Station %d has 0 passengers left to arrive\n", current);
                printf("    Train %d is at Station %d and has %d/%d passengers\n", id, current, on_board, cap);
                printf("        <Nothing more to do>...\n");
                printf("    Train %d is at Station %d and has %d/%d passengers\n", id, current, on_board, cap);
                printf("    Station %d has 0 passengers left to arrive\n", current);
                pthread_mutex_unlock(&stdout_mutex);
            } else {
                to_drop = on_board < needed ? on_board : needed;

                pthread_mutex_lock(&stdout_mutex);
                printf("    Station %d has %d passengers left to arrive\n", current, needed);
                printf("    Train %d is at Station %d and has %d/%d passengers\n", id, current, on_board, cap);
                printf("        Unloading passengers...\n");
                pthread_mutex_unlock(&stdout_mutex);

                do_sleep(to_drop);
                on_board -= to_drop;
                station_destination[current] -= to_drop;

                pthread_mutex_lock(&stdout_mutex);
                printf("    Train %d is at Station %d and has %d/%d passengers\n", id, current, on_board, cap);
                printf("    Station %d has %d passengers left to arrive\n", current, station_destination[current]);
                pthread_mutex_unlock(&stdout_mutex);
            }
        }

        pthread_mutex_lock(&stdout_mutex);
        printf("Train %d LEAVES Station %d\n", id, current);
        pthread_mutex_unlock(&stdout_mutex);
        pthread_mutex_unlock(&station_mutex[current]);

        sleep(1); // travel time

        // Determine next station
        if (on_board == 0) {
            // Head back to station 0
            if (current == 0) continue; // already there, loop will re-enter
            direction = -1;
        } else {
            // Keep going forward
            direction = 1;
        }

        int next = current + direction;
        if (next < 0) next = 0;
        if (next >= NUM_STATIONS) {
            // Went past end, turn around
            direction = -1;
            next = current - 1;
        }
        current = next;
    }

    return NULL;
}

int main(void) {
    pthread_mutex_init(&stdout_mutex, NULL);
    for (int i = 0; i < NUM_STATIONS; i++)
        pthread_mutex_init(&station_mutex[i], NULL);

    Train trains[2] = {{0, TRAIN0_CAP}, {1, TRAIN1_CAP}};
    pthread_t threads[2];

    pthread_create(&threads[0], NULL, train_thread, &trains[0]);
    pthread_create(&threads[1], NULL, train_thread, &trains[1]);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);

    pthread_mutex_destroy(&stdout_mutex);
    for (int i = 0; i < NUM_STATIONS; i++)
        pthread_mutex_destroy(&station_mutex[i]);

    return 0;
}
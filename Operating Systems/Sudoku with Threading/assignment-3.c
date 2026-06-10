/*
* Andrei Ciceu
* 251355626
* CS3305 Assignment 3
* March 10, 2026
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 9

int grid[SIZE][SIZE];

// Struct to pass data to each thread
typedef struct {
    int thread_num;
    int index;       // row, column, or subgrid index (0-8)
    int type;        // 0 = subgrid, 1 = row, 2 = column
    int is_valid;    // result: 1 = valid, 0 = invalid
} ThreadData;

ThreadData thread_data[27];

// Check a single row
void *check_row(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int row = data->index;
    int seen[SIZE + 1] = {0};

    for (int col = 0; col < SIZE; col++) {
        int val = grid[row][col];
        if (seen[val]) {
            data->is_valid = 0;
            pthread_exit(NULL);
        }
        seen[val] = 1;
    }

    data->is_valid = 1;
    pthread_exit(NULL);
}

// Check a single column
void *check_column(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int col = data->index;
    int seen[SIZE + 1] = {0};

    for (int row = 0; row < SIZE; row++) {
        int val = grid[row][col];
        if (seen[val]) {
            data->is_valid = 0;
            pthread_exit(NULL);
        }
        seen[val] = 1;
    }

    data->is_valid = 1;
    pthread_exit(NULL);
}

// Check a single 3x3 subgrid
void *check_subgrid(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int subgrid = data->index;

    // Determine top-left corner of this subgrid
    int start_row = (subgrid / 3) * 3;
    int start_col = (subgrid % 3) * 3;

    int seen[SIZE + 1] = {0};

    for (int r = start_row; r < start_row + 3; r++) {
        for (int c = start_col; c < start_col + 3; c++) {
            int val = grid[r][c];
            if (seen[val]) {
                data->is_valid = 0;
                pthread_exit(NULL);
            }
            seen[val] = 1;
        }
    }

    data->is_valid = 1;
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <sudoku_file>\n", argv[0]);
        return 1;
    }

    // Read the grid from file
    FILE *file = fopen(argv[1], "r");
    if (!file) {
        fprintf(stderr, "Error: could not open file '%s'\n", argv[1]);
        return 1;
    }

    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            fscanf(file, "%d", &grid[i][j]);

    fclose(file);

    pthread_t threads[27];
    int thread_num = 1;

    // Threads 1-9: subgrids
    for (int i = 0; i < 9; i++) {
        thread_data[i].thread_num = thread_num++;
        thread_data[i].index = i;
        thread_data[i].type = 0;
        pthread_create(&threads[i], NULL, check_subgrid, &thread_data[i]);
    }

    // Threads 10-18: rows
    for (int i = 0; i < 9; i++) {
        thread_data[9 + i].thread_num = thread_num++;
        thread_data[9 + i].index = i;
        thread_data[9 + i].type = 1;
        pthread_create(&threads[9 + i], NULL, check_row, &thread_data[9 + i]);
    }

    // Threads 19-27: columns
    for (int i = 0; i < 9; i++) {
        thread_data[18 + i].thread_num = thread_num++;
        thread_data[18 + i].index = i;
        thread_data[18 + i].type = 2;
        pthread_create(&threads[18 + i], NULL, check_column, &thread_data[18 + i]);
    }

    // Join all threads and collect results
    for (int i = 0; i < 27; i++)
        pthread_join(threads[i], NULL);

    // Print results
    int overall_valid = 1;

    for (int i = 0; i < 27; i++) {
        ThreadData *d = &thread_data[i];
        const char *validity = d->is_valid ? "is valid" : "is INVALID";

        if (d->type == 0)
            printf("Thread # %d (subgrid %d) %s\n", d->thread_num, d->index + 1, validity);
        else if (d->type == 1)
            printf("Thread # %d (row %d) %s\n", d->thread_num, d->index + 1, validity);
        else
            printf("Thread # %d (column %d) %s\n", d->thread_num, d->index + 1, validity);

        if (!d->is_valid)
            overall_valid = 0;
    }

    // Extract just the filename for the final message
    const char *filename = argv[1];

    if (overall_valid)
        printf("%s contains a valid solution\n", filename);
    else
        printf("%s contains an INVALID solution\n", filename);

    return 0;
}
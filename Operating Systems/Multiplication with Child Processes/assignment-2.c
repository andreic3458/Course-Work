/*
* Andrei Ciceu
* 251355626
* CS3305 Assignment 2
* February 24, 2026
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>


/* Pipe file descriptors */
/* parent_to_child[0] = read end (child reads), parent_to_child[1] = write end (parent writes) */
/* child_to_parent[0] = read end (parent reads), child_to_parent[1] = write end (child writes) */
int parent_to_child[2];
int child_to_parent[2];

/*
 * Parent side: send two values to child, receive product.
 * Prints communication messages.
 */
long parent_multiply(long x, long y, pid_t parent_pid, pid_t child_pid) {
    long result;

    printf("Parent (PID %d): Sending %ld to child\n", parent_pid, x);
    write(parent_to_child[1], &x, sizeof(long));

    printf("Parent (PID %d): Sending %ld to child\n", parent_pid, y);
    write(parent_to_child[1], &y, sizeof(long));

    read(child_to_parent[0], &result, sizeof(long));
    printf("Parent (PID %d): Received %ld from child\n", parent_pid, result);

    return result;
}

/*
 * Child side: receive two values, multiply, send result back.
 * Prints communication messages.
 */
void child_multiply(pid_t parent_pid, pid_t child_pid) {
    long x, y, result;

    read(parent_to_child[0], &x, sizeof(long));
    printf("Child (PID %d): Received %ld from parent\n", child_pid, x);

    read(parent_to_child[0], &y, sizeof(long));
    printf("Child (PID %d): Received %ld from parent\n", child_pid, y);

    result = x * y;
    printf("Child (PID %d): Sending %ld to parent\n", child_pid, result);
    write(child_to_parent[1], &result, sizeof(long));
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <integer 1000-9999> <integer 1000-9999>\n", argv[0]);
        exit(1);
    }

    int num1 = atoi(argv[1]);
    int num2 = atoi(argv[2]);

    /* Validate range */
    if (num1 < 1000 || num1 > 9999 || num2 < 1000 || num2 > 9999) {
        fprintf(stderr, "Error: Both integers must be in the range 1000-9999.\n");
        exit(1);
    }

    printf("Your integers are %d %d\n", num1, num2);

    /* Decompose */
    long a1 = num1 / 100;
    long a2 = num1 % 100;
    long b1 = num2 / 100;
    long b2 = num2 % 100;

    /* Create pipes */
    if (pipe(parent_to_child) < 0 || pipe(child_to_parent) < 0) {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        /* ---- CHILD PROCESS ---- */
        pid_t child_pid = getpid();
        pid_t parent_pid = getppid();

        /* Close unused ends */
        close(parent_to_child[1]);
        close(child_to_parent[0]);

        /* Perform 4 multiplications as directed */
        child_multiply(parent_pid, child_pid); /* A = a1 * b1 */
        child_multiply(parent_pid, child_pid); /* B = a2 * b1 */
        child_multiply(parent_pid, child_pid); /* C = a1 * b2 */
        child_multiply(parent_pid, child_pid); /* D = a2 * b2 */

        close(parent_to_child[0]);
        close(child_to_parent[1]);
        exit(0);

    } else {
        /* ---- PARENT PROCESS ---- */
        pid_t parent_pid = getpid();
        pid_t child_pid = pid;

        printf("Parent (PID %d): created child (PID %d)\n\n", parent_pid, child_pid);

        /* Close unused ends */
        close(parent_to_child[0]);
        close(child_to_parent[1]);

        /* --- Calculate X = A * 10^4 --- */
        printf("###\n# Calculating X\n###\n");
        long A = parent_multiply(a1, b1, parent_pid, child_pid);
        long X = A * 10000L;
        printf("\n");

        /* --- Calculate Y = (B + C) * 10^2 --- */
        printf("###\n# Calculating Y\n###\n");
        long B = parent_multiply(a2, b1, parent_pid, child_pid);
        long C = parent_multiply(a1, b2, parent_pid, child_pid);
        long Y = (B + C) * 100L;
        printf("\n");

        /* --- Calculate Z = D * 10^0 --- */
        printf("###\n# Calculating Z\n###\n");
        long D = parent_multiply(a2, b2, parent_pid, child_pid);
        long Z = D;
        printf("\n");

        close(parent_to_child[1]);
        close(child_to_parent[0]);

        wait(NULL);

        long result = X + Y + Z;
        printf("%d*%d == %ld + %ld + %ld == %ld\n",
            num1, num2, X, Y, Z, result);
    }

    return 0;
}
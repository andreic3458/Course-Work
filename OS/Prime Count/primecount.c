/*
* Andrei Ciceu 
* 251355626
* CS3305 Assignment 1
* 01/27/2025
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//Main function
//handles all of the code involving printing, system and function calls
int main(int argc, char const *argv[])
{
    //Instantiating variables
    int flag = atoi(argv[1]);
    int min = atoi(argv[2]);
    int max = atoi(argv[3]);
    double quarter = (max - min) / 4;
    long sum = 0;
    int count = 0;

    //Error handling for command line arguments
    if (argc-1 != 3){
        printf("Error: Invalid number of parameters\n");
        return 0;
    } else if (max <= min){
        printf("Error: Max is smaller than min\n");
        return 0;
    }

    //Print initial process id of parent process
    printf("Process id: %d\n", getpid());

    //When flag = 0, find all prime in serial
    if (flag == 0){
        int newMin = min;
        int newMax = min + quarter;
        primeCount(newMin, newMax, &sum, &count);
        printf("pid: %d, ppid %d - Count and sum of prime numbers between %d and %d are %d and %ld\n", getpid(), getppid(), newMin, newMax, count, sum);
        newMin = min + quarter;
        newMax = min + (2 * quarter);
        primeCount(newMin, newMax, &sum, &count);
        printf("pid: %d, ppid %d - Count and sum of prime numbers between %d and %d are %d and %ld\n", getpid(), getppid(), newMin, newMax, count, sum);
        newMin = min + (2 * quarter);
        newMax = min + (3 * quarter);
        primeCount(newMin, newMax, &sum, &count);
        printf("pid: %d, ppid %d - Count and sum of prime numbers between %d and %d are %d and %ld\n", getpid(), getppid(), newMin, newMax, count, sum);
        newMin = min + (3 * quarter);
        newMax = max;
        primeCount(newMin, newMax, &sum, &count);
        printf("pid: %d, ppid %d - Count and sum of prime numbers between %d and %d are %d and %ld\n", getpid(), getppid(), newMin, newMax, count, sum);

    //When flag != 0, find all prime in parallel using fork()
    }else if (flag != 0){
        for (int i = 0; i < 4; i++){
            pid_t pid = fork();
            if (pid == 0){
                int newMin = min + (quarter * i);
                int newMax = min + (quarter * (i+1));
                primeCount(newMin, newMax, &sum, &count);
                printf("pid: %d, ppid %d - Count and sum of prime numbers between %d and %d are %d and %ld\n", getpid(), getppid(), newMin, newMax, count, sum);
                _exit(0);
            }
        }

        for (int i = 0; i < 4; i++) {
            wait(NULL);
        }
    }

    

    return 0;
}

/*Code modified from https://www.programiz.com/c-programming/examples/prime-number-intervals
* primeCount performs count of primes in range provided and prints the count
*/
void primeCount(int min, int max, long *sum, int *count){
    int i, flag;
    *sum = 0;
    *count = 0;

    // iteration until min is not equal to high
    while (min < max) {
        flag = 0;

        // ignore numbers less than 2
        if (min <= 1) {
            min++;
            continue;
        }

        // if min is a non-prime number, flag will be 1
        for (i = 2; i <= min / 2; i++) {

            if (min % i == 0) {
                flag = 1;
                break;
            }
        }

        if (flag == 0){
            (*count)++;
            (*sum) += min;
        }
        // to check prime for the next number
        // increase min by 1
        min++;
    }

    return;
}




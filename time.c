#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>

typedef struct timetime{
    unsigned long sec, nsec;
}Time;

int main(int argc, const char *argv[]){

    /* print time in format: 2009‐08‐10 18:17:54.811 */
    time_t timer1;
    char buffer1[30];
    struct tm* tm_info1;
    time(&timer1);
    tm_info1 = localtime(&timer1);
    strftime(buffer1, 30, "%Y-%m-%d %H:%M:%S", tm_info1);

    Time start, end;
    int s, e;
    if((s = syscall(334, &start.sec, &start.nsec)) != 1){
        perror("syscall 334 failed");
        exit(1);
    }
    
    
    int n = atoi(argv[1]);
    for(int t = 0; t < n; t++){
        volatile unsigned long i;
        for(i = 0; i < 1000000UL; i++);
    }

    int pid = getpid();
    time_t timer2;
    char buffer2[30];
    struct tm* tm_info2;
    time(&timer2);
    tm_info2 = localtime(&timer2);
    strftime(buffer2, 30, "%Y-%m-%d %H:%M:%S", tm_info2);
    //printf("pid %d : time.o start at |%s| and finish at time |%s|\n", getpid(), buffer1, buffer2);


    int c, d;
    if((e = syscall(334, &end.sec, &end.nsec)) != 1){
        perror("syscall 334 failed");
        exit(1);
    }

    char buffer_k[200];
    sprintf(buffer_k, "[Project1] %d %lu.%09lu %lu.%09lu", getpid(), start.sec, start.nsec, end.sec, end.nsec);
    int p;
    if((p = syscall(333, buffer_k) != 1)){
        perror("syscall 333 failed");
        exit(1);
    }

    return 0;
}


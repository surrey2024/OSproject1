#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <errno.h>
#include <sched.h>
#include <assert.h>

int p_num, R[1000], T[1000], P[1000];
int pid[1000];
int tmp_s[1000];
char N[1000][33];

void err_sys(char *a){ perror(a); exit(1); }

void swap(int *xp, int *yp){ int temp = *xp; *xp = *yp; *yp = temp; }

void rank(){ /* run it after soting by ready time */
    for(int i = 0; i < p_num; i++){
        int rnk=0;
        for(int z = 0; z < p_num; z++)
            if(tmp_s[z] > tmp_s[i]) rnk++;
        P[i]=rnk;
    }
}

void print_affinity(){
    cpu_set_t mask;
    long nproc, i;
    if(sched_getaffinity(0, sizeof(cpu_set_t), &mask) == -1){
        err_sys("sched_getaffinity fail");
    }
    else{
        nproc = sysconf(_SC_NPROCESSORS_ONLN);
        printf("sched_getaffinity = ");
        for(i = 0; i < nproc; i++)
            printf("%d ", CPU_ISSET(i, &mask));
        printf("\n");
    }
}

void parent_fork(int i){

    //fprintf(stderr, "parent fork: %s\n", N[i]);
    if((pid[i] = fork()) < 0){
        err_sys("fork error");
    }
    else if(pid[i] == 0){ /* child */
        //print_affinity();
        char tmp[100];
        sprintf(tmp, "%d", T[i]);
        execlp("./time.o", "./time.o", tmp, (char *)0);
    }
    else{
        /* set child CPU */ 
        cpu_set_t my_set;
        CPU_ZERO(&my_set);
        CPU_SET(0, &my_set);
        sched_setaffinity(pid[i], sizeof(cpu_set_t), &my_set);
        /* set child priority */
        struct sched_param param;
        param.sched_priority = 2 + P[i];
        if(sched_setparam(pid[i], &param) == -1){
            err_sys("child set error");
        }
    }	

}

void parent_wait(){
    int finished = 0, arr_f[1000] = {};
    do {
        for(int i = 0; i < p_num; i++){
            //for(int i = p_num - 1 ; i >= 0; i--){
            if(arr_f[i] == 1) continue;
            int wpid = 0, status = 0;
            wpid = waitpid(pid[i], &status, WNOHANG);
            if(wpid != 0){
                finished++;
                arr_f[i] = 1;
                //printf("end waiting %s\n", N[i]);
            }
        }
    } while(finished < p_num);
}

void PSJF(){
    
    for(int i = 0; i < p_num-1; i++)
        for(int j = 0; j < p_num-i-1; j++)
            if(R[j] > R[j+1]){swap(&R[j], &R[j+1]);swap(&T[j], &T[j+1]); \
                char tmp[33]; strcpy(tmp, N[j]); strcpy(N[j], N[j+1]); strcpy(N[j+1], tmp);}

    for(int i = 0; i < p_num; i++) P[i] = i;

    int rest_time[101], is_finish = 0, bang, bong;
    for(int i = 0; i < p_num; i++){
        rest_time[i] = T[i];
        for(int j = 0; j < i; j++){
            if(j < is_finish) continue;
            bong = rest_time[j];
            rest_time[j] -= (R[i]-bang);
            if(rest_time[j] <= 0)
                bang += bong, is_finish++;
            else
                bang = R[i];
        }
        //printf("Check 1\n");
        bang = R[i];
        for(int j = i-1; j >= is_finish; j--){
            if(rest_time[j] > rest_time[j+1]){
                int temp = rest_time[j];
                rest_time[j] = rest_time[j+1];
                rest_time[j+1] = temp;
                //swap(&R[j], &R[j+1]);
                //swap(&T[j], &T[j+1]);
                temp = P[j];
                P[j] = P[j+1];
                P[j+1] = temp;
                //char tmp[33]; strcpy(tmp, N[j]); strcpy(N[j], N[j+1]); strcpy(N[j+1], tmp);
            }
        }
        //printf("Check 2\n");
        for(int j = 0; j <= i; j++){
            //printf("The %d one is %s, P[%d] = %d, remain time = %d.\n",j+1,N[P[j]],j,P[j],rest_time[j]);	
        }
    }
    //printf("After sorting:\n\n");
    for(int i = 0; i < p_num; i++){
        //printf("%s %d %d\n", N[i], R[i], T[i]);
        tmp_s[i] = T[i];
    }
    //printf("\n\n");
    int temp_P[101];
    for(int i = 0; i < p_num; i++)
        temp_P[P[i]] = i;
    for(int i = 0; i < p_num; i++){
        P[i] = p_num - temp_P[i];
        //printf("P[%d] = %d.\n",i,P[i]);
    }
   
    int time_count = 0;
    for(int i = 0; i < p_num; i++){
        int ptime = (i == 0) ? 0 : R[i-1];
        for(int j = 0; j < R[i] - ptime; j++){
            volatile unsigned long k;
            for(k=0;k<1000000UL;k++);
            time_count++;
        }
        parent_fork(i);
    }
    parent_wait();
}

void SJF(){

    for(int i = 0; i < p_num-1; i++)
        for(int j = 0; j < p_num-i-1; j++)
            if(R[j] > R[j+1] || (R[j] == R[j+1]&&T[j] > T[j+1])){ swap(&R[j], &R[j+1]); swap(&T[j], &T[j+1]); \
                char tmp[33]; strcpy(tmp, N[j]); strcpy(N[j], N[j+1]); strcpy(N[j+1], tmp);}

    //printf("After sorting:\n\n");
    for(int i = 0; i < p_num; i++){
        //printf("%s %d %d\n", N[i], R[i], T[i]);
        tmp_s[i] = T[i];
    }
    //rank();
    int temp_P[101];
    for(int i = 0; i < p_num; i++)
        P[i] = i;
    /*
    for(int j = 0;j < p_num; j++)
        printf("The %d one is %s, P[%d] = %d.\n",j+1,N[P[j]],j,P[j]);	
    */
    int point,finish_time = R[0];
    for(int i = 0; i < p_num; i++){
        point = i;
        //printf("------------------------------------------------\n");
        finish_time += T[P[i]];
        //printf("Finish time is %d after process %d is done.\n",finish_time, P[i]);
        for(int j = i ; j < p_num; j++){
            if(R[P[j]] > finish_time) break;
            point++;
        }
        for(int j = point-1; j >= i + 1; j--)
            for(int k = i + 1; k < j; k++)
                if(T[P[k]] > T[P[k+1]]) swap(&P[k],&P[k+1]);

        //for(int j = 0;j < p_num; j++)
            //printf("The %d one is %s, P[%d] = %d.\n",j+1,N[P[j]],j,P[j]);
    }

    for(int i = 0; i < p_num; i++)
        temp_P[P[i]] = i;
    for(int i = 0; i < p_num; i++){
        P[i] = p_num - temp_P[i];
        //printf("P[%d] = %d.\n",i,P[i]);
    }
    //printf("\n\n");
    
    int time_count = 0;
    for(int i = 0; i < p_num; i++){
        int ptime = (i == 0) ? 0 : R[i-1];
        for(int j = 0; j < R[i] - ptime; j++){
            volatile unsigned long k;
            for(k=0;k<1000000UL;k++);
            time_count++;
        }
        parent_fork(i);
    }
    parent_wait();
}

void FIFO(){

    for(int i = 0; i < p_num-1; i++)
        for(int j = 0; j < p_num-i-1; j++)
            if(R[j] > R[j+1]){ swap(&R[j], &R[j+1]); swap(&T[j], &T[j+1]); \
                char tmp[33]; strcpy(tmp, N[j]); strcpy(N[j], N[j+1]); strcpy(N[j+1], tmp);}

    //printf("After sorting:\n\n");
    for(int i = 0; i < p_num; i++){
        P[i] = p_num - 1 - i;
        //printf("%s %d %d\n", N[i], R[i], T[i]);
    }
    //printf("\n\n");

    /* Set scheduler */
    struct sched_param param;
    param.sched_priority = 1;
    if(sched_setscheduler(0, SCHED_FIFO, &param) == -1){
        err_sys("set schedluer failed");
    }
    if(sched_setparam(0, &param) == -1){
        err_sys("parent set error");
    }
    int time_count = 0;
    for(int i = 0; i < p_num; i++){

        int ptime = (i == 0) ? 0 : R[i-1];
        for(int j = 0; j < R[i] - ptime; j++){
            volatile unsigned long k;
            for(k=0;k<1000000UL;k++);
            time_count++;
        }
        parent_fork(i);
    }
    parent_wait();
    
}

void RR(){
    for(int i = 0; i < p_num-1; i++)
        for(int j = 0; j < p_num-i-1; j++)
            if(R[j] > R[j+1]){ swap(&R[j], &R[j+1]); swap(&T[j], &T[j+1]); \
                char tmp[33]; strcpy(tmp, N[j]); strcpy(N[j], N[j+1]); strcpy(N[j+1], tmp);}

    //printf("After sorting:\n\n");
    for(int i = 0; i < p_num; i++){
        P[i] = p_num - 1 - i;
        //printf("%s %d %d\n", N[i], R[i], T[i]);
    }
    //printf("\n\n");

    //for(int i = 0; i < p_num; i++) P[i] = i;
    /* Set scheduler */
    struct sched_param param;
    param.sched_priority = 1;
    if(sched_setscheduler(0, SCHED_FIFO, &param) == -1){
        err_sys("set schedluer failed");
    }
    if(sched_setparam(0, &param) == -1){
        err_sys("parent set error");
    }
    int time_count = 0, start_time = R[0], head = 0, tail = 0;
    int queue[1001],arr_f[1001] = {};
    for(int i = 0; i < p_num; i++){

        int ptime = (i == 0) ? 0 : R[i-1];
        for(int j = 0; j < R[i] - ptime; j++){
            
            //if(arr_f[queue[kk]] == 1) continue;
            int wpid = 0, status = 0;
            if(head != tail){
            	wpid = waitpid(pid[queue[head]], &status, WNOHANG);
            	if(wpid != 0){
            	    start_time = time_count;
            	    arr_f[queue[head++]] = 1;
                    // printf("end waiting %s\n", N[queue[head - 1]]);
            	}	
            }
        
            if(head == tail) start_time = time_count;
            if(time_count-start_time == 500){
                int temp = queue[head];
                start_time = time_count;
                //printf("After Round-Robin:\nThe sequence of process is [");
                for(int k = head; k < tail-1; k++){
                    queue[k] = queue[k+1];
                    //printf("%d ",queue[k]);
                }
                queue[tail - 1] = temp;
                //printf("%d]\n",queue[tail - 1]);
                for(int k = head; k < tail; k++){
                    P[queue[k]] = p_num - 1 - k; 
                }
            	param.sched_priority = 2 + P[queue[tail - 1]] - 1;
            	if(sched_setparam(pid[queue[tail - 1]], &param) == -1){
            	    err_sys("child set error");
            	}
                for(int k = head; k < tail; k++){
                    param.sched_priority = 2 + P[queue[k]];
                    if(sched_setparam(pid[queue[k]], &param) == -1){
                        err_sys("child set error");
                    }

                }
            }
            volatile unsigned long k;
            for(k=0;k<1000000UL;k++);
            time_count++;
        }
    	//fprintf(stderr, "parent fork: %s\n", N[i]);
    	if((pid[i] = fork()) < 0){
    	    err_sys("fork error");
    	}
    	else if(pid[i] == 0){ /* child */
            /* Reset scheduler */
            //struct sched_param param;
            //print_affinity();
            char tmp[100];
            sprintf(tmp, "%d", T[i]);
            //fprintf(stderr, "I start at time %d, p=%d.\n",time_count, 2 + P[i]);
            execlp("./time.o", "./time.o", tmp, (char *)0);
    	}
    	else{

            queue[tail++] = i;
            /* Reset CPU */ 
            cpu_set_t my_set;
            CPU_ZERO(&my_set);
            CPU_SET(0, &my_set);
            sched_setaffinity(pid[i], sizeof(cpu_set_t), &my_set);
            /* set child priority */
            param.sched_priority = 2 + P[i];
            if(sched_setparam(pid[i], &param) == -1){
                err_sys("child set error");
            }
    	}	
		
    }
    //parent_wait();
    do {
        int wpid = 0, status = 0;
        wpid = waitpid(pid[queue[head]], &status, WNOHANG);
        if(head != tail){
            if(wpid != 0)
                {
                    if(wpid == -1) err_sys("wait child error!");
                    start_time = time_count;
        	    arr_f[queue[head++]] = 1;
        	    //printf("end waiting %s\n", N[queue[head - 1]]);
        	}
        }
        if(head == tail) start_time = time_count;
        if(time_count-start_time == 500){
            int temp = queue[head];
            start_time = time_count;
            //printf("After Round-Robin:\nThe sequence of process is [");
            for(int k = head; k < tail-1; k++){
                queue[k] = queue[k+1];
                // printf("%d ",queue[k]);
            }
            queue[tail - 1] = temp;
            //printf("%d]\n",queue[tail - 1]);
            for(int k = head; k < tail; k++){
                P[queue[k]] = p_num - 1 - k; 
            }
            param.sched_priority = 2 + P[queue[tail - 1]] - 1;
            if(sched_setparam(pid[queue[tail - 1]], &param) == -1){
                err_sys("child set error");
            }
            for(int k = head; k < tail; k++){
                param.sched_priority = 2 + P[queue[k]];
                if(sched_setparam(pid[queue[k]], &param) == -1){
                    err_sys("child set error");
                }
            }
			
        }
        volatile unsigned long k;
        for(k=0;k<1000000UL;k++);
        time_count++;
    } while(head != p_num || head != tail);
}
int main(int argc, const char *argv[]){
    //print_affinity();

    char type[20];
    scanf("%s", type);
    scanf("%d",&p_num);
    for(int i = 0; i < p_num; i++){
        scanf("%s%d%d",N[i],&R[i],&T[i]);
        P[i] = i;
    }
    
    /*
    printf("%s %d\n", type, p_num);
    for(int i = 0; i < p_num; i++){
        printf("%s %d %d\n", N[i], R[i], T[i]);
    }
    */
    
    /* Set CPU */ 
    cpu_set_t my_set;        /* Define your cpu_set bit mask. */
    CPU_ZERO(&my_set);       /* Initialize it all to 0, i.e. no CPUs selected. */
    CPU_SET(1, &my_set);     /* set the bit that represents core 2. */
    sched_setaffinity(0, sizeof(cpu_set_t), &my_set);
    //print_affinity();

    /* Set scheduler */
    struct sched_param param;
    param.sched_priority = 1;
    if(sched_setscheduler(0, SCHED_FIFO, &param) == -1){
        err_sys("set schedluer failed");
    }
    if(sched_setparam(0, &param) == -1){
        err_sys("parent set error");
    }

    switch(type[0]){
    case 'F':
        FIFO();
        break;
    case 'S':
        SJF();
        break;
    case 'P':
        PSJF();
        break;
    case 'R':
        RR();
        break;
    default:
        puts("Not valid input");
        exit(1);
    }
    for(int i = 0; i < p_num; i++){
        printf("%s %d\n", N[i], pid[i]);
    }
    return 0;
}

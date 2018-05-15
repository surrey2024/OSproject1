#include <linux/module.h>
#include <linux/time.h>
#include <linux/init.h>
#include <linux/linkage.h>
#include <linux/kernel.h>

asmlinkage int myshow_time(unsigned long *a, unsigned long *b){
    struct timespec nowtime;
    getnstimeofday(&nowtime);
    *a = nowtime.tv_sec;
    *b = nowtime.tv_nsec;
    return 1;
}

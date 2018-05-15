#include <linux/linkage.h>
#include <linux/kernel.h>

asmlinkage int sys_my_add(char *s)
{
    printk("%s\n",s);
    return 1;
}


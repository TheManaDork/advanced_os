#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
// #include <stdbool.h>
#include "lottery.h"

#define MISC_NAME "lottery"
#define FALSE 0
#define TRUE 1

// Demo cmds
#define DEMO_IOC_MAGIC 'k'
#define DEMO_IOC_SET_VAL _IOW(DEMO_IOC_MAGIC, 1, int)
#define DEMO_IOC_GET_VAL _IOW(DEMO_IOC_MAGIC, 2, int)
// lottery cmds (defined in lottery.h)
// #define LOTTERY_MAGIC 'L'
// #define LOTTERY_REGISTER   _IOW(LOTTERY_MAGIC, 1, struct lottery_struct)
// #define LOTTERY_UNREGISTER _IOW(LOTTERY_MAGIC, 2, struct lottery_struct)

// 2 CPU's

static int stored_value = 42;
static struct lottery_struct cpu1[500];
static int index1 = 0;
static int weight1 = 0;
static struct lottery_struct cpu2[500];
static int index2 = 0;
static int weight2 = 0;

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lottery kernel module");

// =========== ioctl setup =============
static long lottery_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	int val;
	struct lottery_struct process;
	int *index;
	int *weight;

	switch(cmd) { 
	case 1/*LOTTERY_REGISTER*/: // arg = struct lottery_struct lottery_info
		pr_info("[LOTTERY_STATUS]; Process registering...\n");

		if(copy_from_user(&process, (struct lottery_struct __user*)arg, sizeof(struct lottery_struct))) {
			return -EFAULT;
		}
		struct lottery_struct *cpuSlot = weight1>weight2 ? &cpu1[index1] : &cpu2[index2];
		index = weight1>weight2 ? &index1 : &index2;
		weight = weight1>weight2 ? &weight1 : &weight2;

		cpuSlot->pid = process.pid;
		cpuSlot->tickets = process.tickets;

		(*weight) += cpuSlot->tickets; 
		(*index)++;
		pr_info("[LOTTERY_STATUS]: Process %ld registered\n", cpuSlot->pid);
	break;
	case LOTTERY_UNREGISTER: // arg = struct lottery_struct lottery_info

		if(copy_from_user(&process, (struct lottery_struct __user*)arg, sizeof(struct lottery_struct))) {
			return -EFAULT;
		}

		bool found = FALSE;
		for(int i = 0; i < index1; i++) {
			if(found) {
				cpu1[i].pid = cpu1[i+1].pid;
				cpu1[i].tickets = cpu1[i+1].tickets;
			}
			if(cpu1[i].pid == process.pid) {
				found = TRUE;
				index1--;
				weight1 -= cpu1[i].tickets;
				cpu1[i].pid = cpu1[i+1].pid;
				cpu1[i].tickets = cpu1[i+1].tickets;
			}
		}
		if(found) break;

		for(int i = 0; i < index2; i++) {
			if(found) {
				cpu2[i].pid = cpu2[i+1].pid;
				cpu2[i].tickets = cpu2[i+1].tickets;
			}
			if(cpu2[i].pid == process.pid) {
				found = TRUE;
				index1--;
				weight1 -= cpu2[i].tickets;
				cpu2[i].pid = cpu2[i+1].pid;
				cpu2[i].tickets = cpu2[i+1].tickets;
			}
		}

		if(!found) {
			pr_err("[LOTTERY_STATUS]: Could not find process %ld in registry\n", process.pid);
		}


	break;
	case DEMO_IOC_SET_VAL:

		if(copy_from_user(&val, (int __user*)arg, sizeof(int))) {
			return -EFAULT;
		}

		stored_value = val;
		pr_info("[LOTTERY_STATUS]: Update stored_value to %d via lottery_ioctl\n", stored_value);

	break;
	case DEMO_IOC_GET_VAL:

		if(copy_to_user((int __user*)arg, &stored_value, sizeof(int))) {
			return -EFAULT;
		}

		pr_info("[LOTTERY_STATUS]: Updated stored_value to %d via lottery_ioct\n", stored_value);

	break;
	default:
		pr_err("[LOTTERY_STATUS]: Invalid IOCTL command 0x%x\n", cmd);
		return -ENOTTY; // invalid IOCTL standard error code
	}

	return 0;
}




// file operations structure. binds sys calls to driver funcs
static const struct file_operations lottery_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = lottery_ioctl,
};

// misc device config structure
static struct miscdevice lottery_dev = {
	.minor = MISC_DYNAMIC_MINOR, // ooh a free minor num
	.name = MISC_NAME, // /dev/MISC_NAME
	.fops = &lottery_fops,
};


static int __init lottery_init(void) {
	// DEVICE INIT START
	int reg = misc_register(&lottery_dev);
	if(reg) {
		pr_err("[LOTTERY_STATUS]: failed to register device /dev/%s (err=%d)\n", MISC_NAME, reg);
		// return reg;
	}

	pr_info("[LOTTERY_STATUS]: Device registered at /dev/%s\n", MISC_NAME);
	// DEVICE INIT END
	return 0;
}

static void __exit lottery_exit(void) {
	// unregister dev on rmmod
	misc_deregister(&lottery_dev);
	pr_info("[LOTTERY_STATUS]: Device /dev/%s unregistered\n", MISC_NAME);
}

module_init(lottery_init);
module_exit(lottery_exit);
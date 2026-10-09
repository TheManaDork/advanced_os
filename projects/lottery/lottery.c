#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>


#include <linux/version.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/pid.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
// #include <stdbool.h>
#include "lottery.h"

#define MISC_NAME "lottery"
#define TIMER_INTERVAL_MS 500


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
static struct lottery_struct *currentProccess1;
static int index1 = 0;
static int weight1 = 0;
static struct lottery_struct cpu2[500];
static struct lottery_struct *currentProccess2;
static int index2 = 0;
static int weight2 = 0;

static struct pid *pid_struct;
static struct task_struct *task;

static int monitor_pid = -1;

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Lottery kernel module");

// =========== ioctl setup =============
static long lottery_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
	int val;
	struct lottery_struct process;
	int *index;
	int *weight;

	switch(cmd) { 
	case LOTTERY_REGISTER: // arg = struct lottery_struct lottery_info

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

		pid_struct = find_get_pid(cpuSlot->pid);
		task = get_pid_task(pid_struct, PIDTYPE_PID);
		put_pid(pid_struct);
		send_sig_info(SIGSTOP, SEND_SIG_PRIV, task);
		put_task_struct(task);

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



// ================== Timer ============
static struct hrtimer lottery_timer;
static struct work_struct lottery_work;
static ktime_t kt_interval;

static enum hrtimer_restart lottery_timer_tick(struct hrtimer *timer) {
	schedule_work(&lottery_work);
	hrtimer_forward_now(timer, kt_interval);
	return HRTIMER_RESTART;
}


static void lottery_monitor(struct work_struct *work) {
	pr_info("[LOTTER_STATUS] running oldest pids\n");
	
	pid_struct = find_get_pid(cpu1[0]->pid);
	task = get_pid_task(pid_struct, PIDTYPE_PID);
	put_pid(pid_struct);
	send_sig_info(SIGCONT, SEND_SIG_PRIV, task);
	put_task_struct(task);

	pid_struct = find_get_pid(cpu1[0]->pid);
	task = get_pid_task(pid_struct, PIDTYPE_PID);
	put_pid(pid_struct);
	send_sig_info(SIGCONT, SEND_SIG_PRIV, task);
	put_task_struct(task);

	return;
}



static int __init lottery_init(void) {
	// DEVICE INIT START
	int reg = misc_register(&lottery_dev);
	if(reg) {
		pr_err("[LOTTERY_STATUS]: failed to register device /dev/%s (err=%d)\n", MISC_NAME, reg);
		return reg;
	}

	pr_info("[LOTTERY_STATUS]: Device registered at /dev/%s\n", MISC_NAME);
	// DEVICE INIT END

	INIT_WORK(&lottery_work, lottery_monitor);
	kt_interval = ktime_set(0, 500*NSEC_PER_MSEC);
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6, 14, 0)
	hrtimer_setup(&lottery_timer, lottery_timer_tick, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
#else
	hrtimer_init(&lottery_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	lottery_timer.function = lottery_timer_tick;
#endif

	pr_info("[LOTTER_STATUS] starting timer");
	hrtimer_start(&lottery_timer, kt_interval, HRTIMER_MODE_REL);

	return 0;
}

static void __exit lottery_exit(void) {
	// unregister dev on rmmod
	misc_deregister(&lottery_dev);
	pr_info("[LOTTERY_STATUS]: Device /dev/%s unregistered\n", MISC_NAME);
}

module_init(lottery_init);
module_exit(lottery_exit);
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kprobes.h>
#include <linux/sched.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Systems Instructor");
MODULE_DESCRIPTION("CR2 Page Fault Verification Probe (Exact Match Filter)");

#define TARGET_ADDRESS 0x500000000041UL

static struct kprobe kp;

static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    unsigned long cr2_val;

    if (strcmp(current->comm, "trigger") == 0) {
        asm volatile("mov %%cr2, %0" : "=r" (cr2_val));
        
        // Only print if the faulting address matches our target page
        if ((cr2_val & ~0xFFFUL) == (TARGET_ADDRESS & ~0xFFFUL)) {
            pr_info("CR2_PROBE: MATCH FOUND! Target address caught in CR2 = 0x%lx\n", cr2_val);
        }
    }
    
    return 0;
}

static int __init cr2_probe_init(void)
{
    int ret;
    kp.pre_handler = handler_pre;
    kp.symbol_name = "handle_mm_fault";

    ret = register_kprobe(&kp);
    if (ret < 0) {
        pr_err("CR2_PROBE: register_kprobe failed, returned %d\n", ret);
        return ret;
    }
    pr_info("CR2_PROBE: Planted kprobe at %p\n", kp.addr);
    return 0;
}

static void __exit cr2_probe_exit(void)
{
    unregister_kprobe(&kp);
    pr_info("CR2_PROBE: Unregistered kprobe.\n");
}

module_init(cr2_probe_init);
module_exit(cr2_probe_exit);

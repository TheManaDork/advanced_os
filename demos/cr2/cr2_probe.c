#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kprobes.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Systems Instructor");
MODULE_DESCRIPTION("CR2 Page Fault Verification Probe");

static struct kprobe kp;

// Pre-handler executed when exc_page_fault is hit
static int handler_pre(struct kprobe *p, struct pt_regs *regs)
{
    unsigned long cr2_val;

    // Read the CR2 control register directly using inline assembly
    asm volatile("mov %%cr2, %0" : "=r" (cr2_val));

    pr_info("CR2_PROBE: Page fault intercepted! CR2 = 0x%lx\n", cr2_val);
    
    // Optional: You can also inspect error codes in regs->dx / error_code if needed
    return 0; // Return 0 to normal execution flow
}

static int __init cr2_probe_init(void)
{
    int ret;
    kp.pre_handler = handler_pre;
    kp.symbol_name = "exc_page_fault"; // Use "do_page_fault" on older kernels (< 5.x)

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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/pagemap.h>
#include <linux/sched.h>

static int entry_handler(struct kretprobe_instance *ri,
                         struct pt_regs *regs)
{
        return 0;
}

static int ret_handler(struct kretprobe_instance *ri,
                       struct pt_regs *regs)
{
        struct folio *folio;
        unsigned long index;

        /*
         * filemap_get_entry(mapping, index)
         *
         * On x86-64, the second argument is in RSI.
         */
        index = regs->si;

        /*
         * kretprobe return value on x86-64 is in RAX.
         */
        folio = (struct folio *)regs->ax;

        if (folio) {
                pr_info("PA-LAB: PAGE CACHE HIT pid=%d comm=%s index=%lu folio=%px\n",
                        current->pid, current->comm, index, folio);
        } else {
                pr_info("PA-LAB: PAGE CACHE MISS pid=%d comm=%s index=%lu\n",
                        current->pid, current->comm, index);
        }

        return 0;
}

static struct kretprobe kp = {
        .handler = ret_handler,
        .entry_handler = entry_handler,
        .maxactive = 64,
        .kp.symbol_name = "filemap_get_entry",
};

static int __init pagecache_probe_init(void)
{
        int ret;

        ret = register_kretprobe(&kp);
        if (ret < 0) {
                pr_err("PA-LAB: register_kretprobe failed: %d\n", ret);
                return ret;
        }

        pr_info("PA-LAB: page-cache probe loaded\n");

        return 0;
}

static void __exit pagecache_probe_exit(void)
{
        unregister_kretprobe(&kp);

        pr_info("PA-LAB: page-cache probe unloaded\n");
}

module_init(pagecache_probe_init);
module_exit(pagecache_probe_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Advanced OS Lab");
MODULE_DESCRIPTION("Observe Linux page-cache lookup results");

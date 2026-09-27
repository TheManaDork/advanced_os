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
        struct address_space *mapping;
        struct inode *inode;
        struct folio *folio;
        unsigned long index;

        /*
         * filemap_get_entry(struct address_space *mapping, pgoff_t index)
         *
         * x86-64 calling convention:
         *   RDI = mapping
         *   RSI = index
         *   RAX = return value
         */
        mapping = (struct address_space *)regs->di;
        index = regs->si;
        folio = (struct folio *)regs->ax;
        if (!mapping)
                return 0;
        inode = mapping->host;
        if (!inode)
                return 0;

        /*
         * Only observe data.bin.
         *
         * data.bin currently has inode 1502784.
         */
        if (inode->i_ino != 1502784)
                return 0;

        if (folio) {
                pr_info("PA-LAB: PAGE CACHE HIT pid=%d index=%lu\n",
                        current->pid, index);
        } else {
                pr_info("PA-LAB: PAGE CACHE MISS pid=%d index=%lu\n",
                        current->pid, index);
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/pagemap.h>
#include <linux/fs.h>
#include <linux/sched.h>

#define DATA_INODE 1502784

struct probe_data {
        struct address_space *mapping;
        unsigned long index;
};

static int entry_handler(struct kretprobe_instance *ri,
                         struct pt_regs *regs)
{
        struct probe_data *data;

        data = (struct probe_data *)ri->data;

        /*
         * filemap_get_entry(struct address_space *mapping, pgoff_t index)
         *
         * x86-64 calling convention:
         *   RDI = mapping
         *   RSI = index
         */
        data->mapping = (struct address_space *)regs->di;
        data->index = regs->si;

        return 0;
}

static int ret_handler(struct kretprobe_instance *ri,
                       struct pt_regs *regs)
{
        struct probe_data *data;
        struct address_space *mapping;
        struct inode *inode;
        struct folio *folio;

        data = (struct probe_data *)ri->data;

        mapping = data->mapping;

        if (!mapping)
                return 0;

        inode = mapping->host;

        if (!inode)
                return 0;

        /*
         * Only observe data.bin.
         */
        if (inode->i_ino != DATA_INODE)
                return 0;

        /*
         * x86-64 return value:
         *   RAX = return value
         */
        folio = (struct folio *)regs->ax;

        if (folio) {
                pr_info("PA-LAB: PAGE CACHE HIT pid=%d index=%lu\n",
                        current->pid, data->index);
        } else {
                pr_info("PA-LAB: PAGE CACHE MISS pid=%d index=%lu\n",
                        current->pid, data->index);
        }

        return 0;
}

static struct kretprobe kp = {
        .handler = ret_handler,
        .entry_handler = entry_handler,
        .data_size = sizeof(struct probe_data),
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

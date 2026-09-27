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
        struct address_space *mapping;
        struct inode *inode;

        data = (struct probe_data *)ri->data;

        /*
         * filemap_get_entry(mapping, index)
         *
         * x86-64:
         *   RDI = mapping
         *   RSI = index
         */
        mapping = (struct address_space *)regs->di;
        data->mapping = mapping;
        data->index = regs->si;

        /*
         * Temporarily report what mapfile is actually looking up.
         */
        if (strncmp(current->comm, "mapfile", TASK_COMM_LEN) == 0) {
                if (mapping && mapping->host) {
                        inode = mapping->host;

                        pr_info("PA-LAB: ENTRY pid=%d index=%lu inode=%lu mapping=%px\n",
                                current->pid,
                                data->index,
                                inode->i_ino,
                                mapping);
                }
        }

        return 0;
}

static int ret_handler(struct kretprobe_instance *ri,
                       struct pt_regs *regs)
{
        struct probe_data *data;
        struct folio *folio;

        data = (struct probe_data *)ri->data;

        /*
         * Only report mapfile.
         */
        if (strncmp(current->comm, "mapfile", TASK_COMM_LEN) != 0)
                return 0;

        folio = (struct folio *)regs->ax;

        if (folio) {
                pr_info("PA-LAB: RETURN HIT pid=%d index=%lu\n",
                        current->pid, data->index);
        } else {
                pr_info("PA-LAB: RETURN MISS pid=%d index=%lu\n",
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

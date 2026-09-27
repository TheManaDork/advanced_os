#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/err.h>

#define DATA_INODE 1502784

/*
 * We only care about the initial lookup performed by filemap_fault().
 *
 * filemap_fault() first calls:
 *
 *     filemap_get_folio(mapping, index)
 *
 * which ultimately calls __filemap_get_folio_mpol() with fgp_flags == 0.
 *
 * If that lookup returns:
 *
 *     real folio       -> page-cache HIT
 *     ERR_PTR(-ENOENT) -> page-cache MISS
 *
 * Calls with FGP_CREAT belong to the subsequent miss/creation path
 * and are ignored.
 */

struct probe_data {
        struct address_space *mapping;
        pgoff_t index;
        unsigned int fgp_flags;
};

static int entry_handler(struct kretprobe_instance *ri,
                         struct pt_regs *regs)
{
        struct probe_data *data;
        struct address_space *mapping;
        pgoff_t index;
        unsigned int fgp_flags;

        data = (struct probe_data *)ri->data;

        /*
         * x86-64 System V calling convention:
         *
         *   RDI = mapping
         *   RSI = index
         *   RDX = fgp_flags
         */
        mapping = (struct address_space *)regs->di;
        index = (pgoff_t)regs->si;
        fgp_flags = (unsigned int)regs->dx;

        data->mapping = mapping;
        data->index = index;
        data->fgp_flags = fgp_flags;

        return 0;
}

static int ret_handler(struct kretprobe_instance *ri,
                       struct pt_regs *regs)
{
        struct probe_data *data;
        struct address_space *mapping;
        struct inode *inode;
        unsigned long ret;

        data = (struct probe_data *)ri->data;

        /*
         * Only classify the initial lookup.
         */
        if (data->fgp_flags != 0)
                return 0;

        mapping = data->mapping;

        if (!mapping || !mapping->host)
                return 0;

        inode = mapping->host;

        /*
         * Only our data.bin.
         */
        if (inode->i_ino != DATA_INODE)
                return 0;

        /*
         * data.bin is one page and therefore index 0.
         */
        if (data->index != 0)
                return 0;

        ret = regs_return_value(regs);

        if (!IS_ERR_VALUE(ret)) {
                pr_info("PA-LAB: PAGE CACHE HIT pid=%d "
                        "inode=%lu index=%lu\n",
                        current->pid,
                        inode->i_ino,
                        (unsigned long)data->index);
        } else if (PTR_ERR((void *)ret) == -ENOENT) {
                pr_info("PA-LAB: PAGE CACHE MISS pid=%d "
                        "inode=%lu index=%lu\n",
                        current->pid,
                        inode->i_ino,
                        (unsigned long)data->index);
        }

        return 0;
}

static struct kretprobe pagecache_probe = {
        .kp.symbol_name = "__filemap_get_folio_mpol",
        .entry_handler = entry_handler,
        .handler = ret_handler,
        .data_size = sizeof(struct probe_data),
        .maxactive = 64,
};

static int __init pagecache_probe_init(void)
{
        int ret;

        ret = register_kretprobe(&pagecache_probe);

        if (ret < 0) {
                pr_err("PA-LAB: register_kretprobe failed: %d\n", ret);
                return ret;
        }

        pr_info("PA-LAB: __filemap_get_folio_mpol probe loaded\n");

        return 0;
}

static void __exit pagecache_probe_exit(void)
{
        unregister_kretprobe(&pagecache_probe);

        pr_info("PA-LAB: __filemap_get_folio_mpol probe unloaded\n");
}

module_init(pagecache_probe_init);
module_exit(pagecache_probe_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Advanced OS Lab");
MODULE_DESCRIPTION("Exact page-cache hit/miss probe for data.bin");

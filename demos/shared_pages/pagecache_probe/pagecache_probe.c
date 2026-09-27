#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/fs.h>
#include <linux/pagemap.h>
#include <linux/err.h>
#include <linux/sched.h>
#include <linux/compiler.h>

#define DATA_INODE 1502784

/*
 * PID of the process that caused the initial page-cache MISS.
 *
 * This is intentionally simple because this probe is designed for
 * the two-process Advanced OS lab experiment.
 */
static pid_t miss_pid = -1;


/*
 * ============================================================
 * Probe #1: __filemap_get_folio_mpol()
 * ============================================================
 *
 * filemap_fault() first calls:
 *
 *     filemap_get_folio(mapping, index)
 *
 * which eventually calls:
 *
 *     __filemap_get_folio_mpol(mapping, index, 0, ...)
 *
 * If the folio is not in the page cache, this lookup returns
 * ERR_PTR(-ENOENT).
 *
 * We use this to identify the initial PAGE CACHE MISS.
 */


/*
 * Per-instance data for the kretprobe.
 */
struct lookup_probe_data {
        struct address_space *mapping;
        pgoff_t index;
        unsigned int fgp_flags;
};


static int lookup_entry_handler(struct kretprobe_instance *ri,
                                struct pt_regs *regs)
{
        struct lookup_probe_data *data;

        data = (struct lookup_probe_data *)ri->data;

        /*
         * x86-64 calling convention:
         *
         * RDI = mapping
         * RSI = index
         * RDX = fgp_flags
         */
        data->mapping = (struct address_space *)regs->di;
        data->index = (pgoff_t)regs->si;
        data->fgp_flags = (unsigned int)regs->dx;

        return 0;
}


static int lookup_ret_handler(struct kretprobe_instance *ri,
                              struct pt_regs *regs)
{
        struct lookup_probe_data *data;
        struct address_space *mapping;
        struct inode *inode;
        unsigned long ret;

        data = (struct lookup_probe_data *)ri->data;

        /*
         * We only care about the initial lookup:
         *
         *     filemap_get_folio()
         *
         * The later FGP_CREAT lookup belongs to the miss/creation
         * path and is deliberately ignored.
         */
        if (data->fgp_flags != 0)
                return 0;

        mapping = data->mapping;

        if (!mapping || !mapping->host)
                return 0;

        inode = mapping->host;

        /*
         * Only monitor our data.bin.
         */
        if (inode->i_ino != DATA_INODE)
                return 0;

        /*
         * data.bin is one page, so we only care about index 0.
         */
        if (data->index != 0)
                return 0;

        ret = regs_return_value(regs);

        /*
         * filemap_get_folio() failed to find the folio.
         */
        if (IS_ERR_VALUE(ret) &&
            PTR_ERR((void *)ret) == -ENOENT) {

                /*
                 * Remember which process caused the miss.
                 *
                 * That process may subsequently enter
                 * filemap_map_pages() and find the page it just
                 * caused to be populated. We don't want to call
                 * that a HIT in this experiment.
                 */
                WRITE_ONCE(miss_pid, current->pid);

                pr_info("PA-LAB: PAGE CACHE MISS pid=%d "
                        "inode=%lu index=%lu\n",
                        current->pid,
                        inode->i_ino,
                        (unsigned long)data->index);
        }

        return 0;
}


/*
 * ============================================================
 * Probe #2: next_uptodate_folio()
 * ============================================================
 *
 * Linux 7.0 filemap_map_pages() contains:
 *
 *     folio = next_uptodate_folio(&xas, mapping, end_pgoff);
 *
 * next_uptodate_folio() walks the mapping's xarray and returns
 * an existing, uptodate folio from the page cache.
 *
 * Therefore, a successful return for our inode/index represents
 * finding the page that is already in the page cache.
 */


/*
 * Per-instance data for the next_uptodate_folio() kretprobe.
 */
struct map_probe_data {
        struct xa_state *xas;
        struct address_space *mapping;
};


static int map_entry_handler(struct kretprobe_instance *ri,
                             struct pt_regs *regs)
{
        struct map_probe_data *data;

        data = (struct map_probe_data *)ri->data;

        /*
         * x86-64 calling convention for:
         *
         *     next_uptodate_folio(xas, mapping, end_pgoff)
         *
         * RDI = xas
         * RSI = mapping
         * RDX = end_pgoff
         */
        data->xas = (struct xa_state *)regs->di;
        data->mapping = (struct address_space *)regs->si;

        return 0;
}


static int map_ret_handler(struct kretprobe_instance *ri,
                           struct pt_regs *regs)
{
        struct map_probe_data *data;
        struct address_space *mapping;
        struct inode *inode;
        struct folio *folio;
        pgoff_t index;

        data = (struct map_probe_data *)ri->data;

        mapping = data->mapping;

        if (!mapping || !mapping->host)
                return 0;

        inode = mapping->host;

        /*
         * Only monitor our data.bin.
         */
        if (inode->i_ino != DATA_INODE)
                return 0;

        /*
         * next_uptodate_folio() returns:
         *
         *     struct folio *  -> found an existing uptodate folio
         *     NULL            -> no suitable folio
         */
        folio = (struct folio *)regs_return_value(regs);

        if (!folio)
                return 0;

        /*
         * For our one-page data.bin experiment, the returned
         * folio must represent index 0.
         */
        index = folio->index;

        if (index != 0)
                return 0;

        /*
         * The process that caused the MISS may subsequently call
         * filemap_map_pages() and find the page it just populated.
         *
         * Do not report that as the HIT we want to demonstrate.
         */
        if (current->pid == READ_ONCE(miss_pid))
                return 0;

        pr_info("PA-LAB: PAGE CACHE HIT pid=%d "
                "inode=%lu index=%lu\n",
                current->pid,
                inode->i_ino,
                (unsigned long)index);

        return 0;
}


/*
 * ============================================================
 * Kretprobe definitions
 * ============================================================
 */

static struct kretprobe lookup_probe = {
        .kp.symbol_name = "__filemap_get_folio_mpol",
        .entry_handler = lookup_entry_handler,
        .handler = lookup_ret_handler,
        .data_size = sizeof(struct lookup_probe_data),
        .maxactive = 64,
};


static struct kretprobe map_probe = {
        .kp.symbol_name = "next_uptodate_folio",
        .entry_handler = map_entry_handler,
        .handler = map_ret_handler,
        .data_size = sizeof(struct map_probe_data),
        .maxactive = 64,
};


/*
 * ============================================================
 * Module initialization
 * ============================================================
 */

static int __init pagecache_probe_init(void)
{
        int ret;

        WRITE_ONCE(miss_pid, -1);

        /*
         * Register the initial page-cache lookup probe.
         */
        ret = register_kretprobe(&lookup_probe);

        if (ret < 0) {
                pr_err("PA-LAB: lookup kretprobe registration failed: %d\n",
                       ret);
                return ret;
        }

        /*
         * Register the map_pages lookup probe.
         */
        ret = register_kretprobe(&map_probe);

        if (ret < 0) {
                pr_err("PA-LAB: next_uptodate_folio kretprobe "
                       "registration failed: %d\n",
                       ret);

                unregister_kretprobe(&lookup_probe);

                return ret;
        }

        pr_info("PA-LAB: page-cache HIT/MISS probes loaded\n");

        return 0;
}


/*
 * ============================================================
 * Module cleanup
 * ============================================================
 */

static void __exit pagecache_probe_exit(void)
{
        unregister_kretprobe(&map_probe);
        unregister_kretprobe(&lookup_probe);

        pr_info("PA-LAB: page-cache HIT/MISS probes unloaded\n");
}


module_init(pagecache_probe_init);
module_exit(pagecache_probe_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Advanced OS Lab");
MODULE_DESCRIPTION("Page-cache HIT/MISS probe for data.bin");

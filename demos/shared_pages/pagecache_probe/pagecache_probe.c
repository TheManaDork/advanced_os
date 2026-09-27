#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kprobes.h>
#include <linux/mm.h>
#include <linux/fs.h>

#define DATA_INODE 1502784

struct probe_data {
        struct vm_fault *vmf;
};

static int entry_handler(struct kretprobe_instance *ri,
                         struct pt_regs *regs)
{
        struct probe_data *data;

        data = (struct probe_data *)ri->data;

        /*
         * filemap_fault(struct vm_fault *vmf)
         *
         * x86-64:
         *   RDI = vmf
         */
        data->vmf = (struct vm_fault *)regs->di;

        return 0;
}

static int ret_handler(struct kretprobe_instance *ri,
                       struct pt_regs *regs)
{
        struct probe_data *data;
        struct vm_fault *vmf;
        struct file *file;
        struct inode *inode;

        data = (struct probe_data *)ri->data;
        vmf = data->vmf;

        if (!vmf || !vmf->vma)
                return 0;

        file = vmf->vma->vm_file;

        if (!file)
                return 0;

        inode = file_inode(file);

        if (!inode)
                return 0;

        /*
         * Only observe data.bin.
         */
        if (inode->i_ino != DATA_INODE)
                return 0;

        pr_info("PA-LAB: FILEMAP_FAULT pid=%d inode=%lu index=%lu ret=0x%lx\n",
                current->pid,
                inode->i_ino,
                vmf->pgoff,
                regs->ax);

        return 0;
}

static struct kretprobe kp = {
        .handler = ret_handler,
        .entry_handler = entry_handler,
        .data_size = sizeof(struct probe_data),
        .maxactive = 64,
        .kp.symbol_name = "filemap_fault",
};

static int __init pagecache_probe_init(void)
{
        int ret;

        ret = register_kretprobe(&kp);

        if (ret < 0) {
                pr_err("PA-LAB: register_kretprobe failed: %d\n", ret);
                return ret;
        }

        pr_info("PA-LAB: filemap_fault probe loaded\n");

        return 0;
}

static void __exit pagecache_probe_exit(void)
{
        unregister_kretprobe(&kp);

        pr_info("PA-LAB: filemap_fault probe unloaded\n");
}

module_init(pagecache_probe_init);
module_exit(pagecache_probe_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Advanced OS Lab");
MODULE_DESCRIPTION("Trace filemap_fault for data.bin");

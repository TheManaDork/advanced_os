#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>

static char secret[8] = {'S','E','E','D','L','a','b','s'};
static struct proc_dir_entry *secret_entry;
static char *secret_buffer;


static int test_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, NULL, pde_data(inode));
}


static ssize_t read_proc(struct file *filp,
                         char __user *buffer,
                         size_t length,
                         loff_t *offset)
{
    /*
     * Copy the secret into the kernel buffer.
     *
     * This preserves the original lab structure, where the
     * secret is copied into secret_buffer before being returned.
     */
    memcpy(secret_buffer, secret, 8);

    /*
     * Return the data to user space.
     */
    if (*offset >= 8)
        return 0;

    if (length > 8 - *offset)
        length = 8 - *offset;

    if (copy_to_user(buffer, secret_buffer + *offset, length))
        return -EFAULT;

    *offset += length;

    return length;
}


static const struct proc_ops test_proc_fops =
{
    .proc_open    = test_proc_open,
    .proc_read    = read_proc,
    .proc_lseek   = seq_lseek,
    .proc_release = single_release,
};


static int __init test_proc_init(void)
{
    /*
     * Print the kernel virtual address of the secret.
     */
    printk(KERN_INFO "secret data address:%px\n", &secret);

    secret_buffer = vmalloc(8);

    if (!secret_buffer)
        return -ENOMEM;

    /*
     * Create:
     *
     *     /proc/secret_data
     */
    secret_entry = proc_create_data("secret_data",
                                    0444,
                                    NULL,
                                    &test_proc_fops,
                                    NULL);

    if (secret_entry)
        return 0;

    vfree(secret_buffer);

    return -ENOMEM;
}


static void __exit test_proc_cleanup(void)
{
    remove_proc_entry("secret_data", NULL);

    vfree(secret_buffer);
}


module_init(test_proc_init);
module_exit(test_proc_cleanup);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel module for the Meltdown side-channel demonstration");

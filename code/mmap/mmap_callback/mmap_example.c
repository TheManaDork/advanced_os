#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define DEVICE_NAME "mmap_example"
#define CLASS_NAME  "mmap_class"
#define BUFFER_SIZE PAGE_SIZE  // 1 page

static int major;
static struct class *mmap_class = NULL;
static struct device *mmap_device = NULL;
static char *kernel_buffer;

static int my_open(struct inode *inode, struct file *file)
{
    return 0;
}

// mmap callback
static int my_mmap(struct file *filp, struct vm_area_struct *vma)
{
    unsigned long phys_addr = virt_to_phys(kernel_buffer);
    unsigned long size = vma->vm_end - vma->vm_start;

    if (size > BUFFER_SIZE)
        return -EINVAL;

    // Map kernel buffer to user space
    if (remap_pfn_range(vma,
                        vma->vm_start,
                        phys_addr >> PAGE_SHIFT,
                        size,
                        vma->vm_page_prot)) {
        return -EAGAIN;
    }

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .mmap = my_mmap,
};

static int __init my_mmap_init(void)
{
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        pr_err("Failed to register device\n");
        return major;
    }

    // Create class
    mmap_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(mmap_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        pr_err("Failed to create class\n");
        return PTR_ERR(mmap_class);
    }

    // Create device node in /dev/
    mmap_device = device_create(mmap_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);
    if (IS_ERR(mmap_device)) {
        class_destroy(mmap_class);
        unregister_chrdev(major, DEVICE_NAME);
        pr_err("Failed to create device\n");
        return PTR_ERR(mmap_device);
    }

    kernel_buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!kernel_buffer) {
        unregister_chrdev(major, DEVICE_NAME);
        return -ENOMEM;
    }

    pr_info("mmap_example loaded: major=%d\n", major);
    return 0;
}

static void __exit my_mmap_exit(void)
{
    pr_info("kernel buffer content: %s\n", kernel_buffer);
    kfree(kernel_buffer);
    device_destroy(mmap_class, MKDEV(major, 0));
    class_destroy(mmap_class);
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("mmap_example unloaded\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("Minimal mmap kernel module");

module_init(my_mmap_init);
module_exit(my_mmap_exit);

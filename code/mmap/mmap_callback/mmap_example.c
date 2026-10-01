#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/io.h>

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
    unsigned long phys_addr;
    unsigned long size = vma->vm_end - vma->vm_start;

    if (size > BUFFER_SIZE)
        return -EINVAL;

    phys_addr = virt_to_phys(kernel_buffer);

    // Map the kernel buffer into user space.
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
    // Allocate one page of kernel memory.
    kernel_buffer = kzalloc(BUFFER_SIZE, GFP_KERNEL);
    if (!kernel_buffer) {
        pr_err("Failed to allocate kernel buffer\n");
        return -ENOMEM;
    }

    // Register the character device.
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        pr_err("Failed to register device\n");
        kfree(kernel_buffer);
        return major;
    }

    // Create the device class.
    // Linux 7.0 class_create() takes only the class name.
    mmap_class = class_create(CLASS_NAME);
    if (IS_ERR(mmap_class)) {
        pr_err("Failed to create class\n");
        unregister_chrdev(major, DEVICE_NAME);
        kfree(kernel_buffer);
        return PTR_ERR(mmap_class);
    }

    // Create the device node in /dev/.
    mmap_device = device_create(mmap_class, NULL,
                                MKDEV(major, 0),
                                NULL, DEVICE_NAME);
    if (IS_ERR(mmap_device)) {
        pr_err("Failed to create device\n");
        class_destroy(mmap_class);
        unregister_chrdev(major, DEVICE_NAME);
        kfree(kernel_buffer);
        return PTR_ERR(mmap_device);
    }

    pr_info("mmap_example loaded: major=%d\n", major);

    return 0;
}

static void __exit my_mmap_exit(void)
{
    pr_info("kernel buffer content: %s\n", kernel_buffer);

    device_destroy(mmap_class, MKDEV(major, 0));
    class_destroy(mmap_class);
    unregister_chrdev(major, DEVICE_NAME);

    kfree(kernel_buffer);

    pr_info("mmap_example unloaded\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("Minimal mmap kernel module");

module_init(my_mmap_init);
module_exit(my_mmap_exit);

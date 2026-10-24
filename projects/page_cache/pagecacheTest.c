#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/pagemap.h> // breaks things
#include <linux/mm.h> // also breaks things?
#include <linux/fs.h>




#define PROC_NAME "pagecache_test"
#define INPUT_SIZE 512

static ssize_t pagecache_write(struct file *file,
                             const char __user *buffer,
                             size_t count,
                             loff_t *pos)
{
    char input[INPUT_SIZE];

    int pid_number;
    unsigned long user_address;
    char filename[256];

    /*
     * Do not allow the input to exceed our buffer.
     */
    if (count >= sizeof(input))
        return -EINVAL;

    /*
     * Copy the string from user space into the kernel buffer.
     */
    if (copy_from_user(input, buffer, count))
        return -EFAULT;

    input[count] = '\0';

    /*
     * Parse:
     *
     *     PID ADDRESS FILE
     *
     * Example:
     *
     *     12345 0x7f1234000000 data.bin
     */
    if (sscanf(input, "%d %lx %255s",
               &pid_number,
               &user_address,
               filename) != 3) {

        pr_err("[PAGECACHE]: invalid input\n");
        pr_info("Usage: echo \"PID ADDRESS FILE\" > /proc/%s\n",
                PROC_NAME);

        return -EINVAL;
    }


    file = filp_open(filename, O_RDONLY | O_LARGEFILE, 0);

    if(IS_ERR(file)) {
    	pr_err("[PAGECACHE] filp_open failed\n");
    	return PTR_ERR(file);
    }


    struct address_space *mapping = file_inode(file)->i_mapping;
    
    struct page *page = find_get_page(mapping, 0);

    if(page == NULL) {
    	pr_err("[PAGECACHE]: get_cache_page: page is NOT in page cache\n");
    	filp_close(file, NULL);
    	return -ENOENT;
    }


    unsigned long pfn = page_to_pfn(page);

    /*
     * Just print the values for demonstration.
     */
		pr_info("=== Cached Page: ========================================= \n");
    pr_info("[PAGECACHE]: PID              = %d\n", pid_number);
    pr_info("[PAGECACHE]: Virtual Address  = 0x%lx\n", user_address);
    // pr_info("[PAGECACHE]: File      = %s\n", filename);
    pr_info("[PAGECACHE]: PFN       = %lu\n", pfn);
    pr_info("[PAGECACHE]: Physical Address = 0x%lx\n",
            pfn << PAGE_SHIFT);
    pr_info("[PAGECACHE]: First 16 bytes:\n");

    char *cachebuffer = kmalloc(17, GFP_ATOMIC);
    int buffersize = 16;
    memset(cachebuffer, 0, sizeof(cachebuffer));
    int bytes_read = kernel_read(file, cachebuffer, buffersize, 0);
    if (bytes_read < 0) {
        pr_err("Error reading file: %ld\n", (long)bytes_read);
    }
    for (int i = 0; i < 16; i++) {
      pr_info("[PAGECACHE]: %02x", (unsigned char)cachebuffer[i]);
    }

    struct pid *pid_struct = find_get_pid(pid_number); 
    if (!pid_struct) {
        pr_err("[PAGECACHE]: get_page: Could not find pid_struct for PID %d\n", pid_number);
        return -ESRCH;
    }

    struct task_struct *task = get_pid_task(pid_struct, PIDTYPE_PID);
    put_pid(pid_struct);

    if (!task) {
        pr_err("[PAGECACHE]: get_page: Could not find task_struct for PID %d\n", pid_number);
        return -ESRCH;
    }

    struct mm_struct *mm = get_task_mm(task);
    if (!mm) {
        pr_err("get_page: Process PID %d has no active mm_struct (kernel thread?)\n", pid_number);
        put_task_struct(task);
        return -EINVAL;
    }

    struct page *page2;
    mmap_read_lock(mm);
    long ret = get_user_pages_remote(mm,
                                user_address,
                                1,
                                FOLL_GET,
                                &page2,
                                NULL);
    mmap_read_unlock(mm);
    if (ret != 1) {
        pr_err("get_page: get_user_pages_remote() failed with error code: %ld\n", ret);
        mmput(mm);
        put_task_struct(task);
        return -EFAULT;
    }

		pfn = page_to_pfn(page2);

		pr_info("=== User Buffer: ========================================= \n");
    pr_info("[PAGECACHE]: PID:               %d\n", pid_number);
    pr_info("[PAGECACHE]: Virtual address:   0x%lx\n", user_address);
    pr_info("[PAGECACHE]: PFN:               %lu\n", pfn);
    pr_info("[PAGECACHE]: Physical address:  0x%lx\n", pfn << PAGE_SHIFT);
    pr_info("[PAGECACHE]: First 16 bytes:\n");

    void *vaddr = kmap_local_page(page2);
    char *userbuffer = kmalloc(17, GFP_ATOMIC);
    // int buffersize = 16;
    memset(userbuffer, 0, sizeof(userbuffer));
    unsigned long offset = user_address & (PAGE_SIZE - 1);
    memcpy(userbuffer, vaddr + offset, buffersize);
    kunmap_local(vaddr);


    for (int i = 0; i < 16; i++) {
      pr_info("[PAGECACHE]: %02x", (unsigned char)userbuffer[i]);
    }

    void *page1_addr = page_address(page);
    void *page2_addr = page_address(page2);

    pr_info("[PAGECACHE]: memcmp() = %d\n", memcmp(page1_addr, page2_addr, PAGE_SIZE));
    // for (int i = 0; i < 16; i++)
        // pr_info(" %02x", (unsigned char)buffer[i]);
    pr_info("========================================\n");

    put_task_struct(task);
    put_page(page);
    put_page(page2);
    filp_close(file, NULL);
    kfree(cachebuffer);
    kfree(userbuffer);
    return count;
}

static const struct proc_ops pagecache_ops = {
    .proc_write = pagecache_write,
};

static int __init page_cache_init(void)
{
    if (proc_create(PROC_NAME, 0666, NULL, &pagecache_ops) == NULL) {
        pr_err("[PAGECACHE]: failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    pr_info("[PAGECACHE]: loaded\n");

    return 0;
}

static void __exit pagecache_exit(void)
{
    remove_proc_entry(PROC_NAME, NULL);

    pr_info("[PAGECACHE]: unloaded\n");
}

module_init(page_cache_init);
module_exit(pagecache_exit);

MODULE_LICENSE("GPL");
// MODULE_AUTHOR("Me");
MODULE_DESCRIPTION("Testing for redundant file copies");

# Demo: Shared Pages and Copy-on-Write

## Overview

In this demo, we will investigate how Linux allows multiple processes to map the same physical memory page, even though the processes have completely independent virtual address spaces.

---

## 1. Background

Every process has its own virtual address space.

For example:

```text
Process A                         Process B

Virtual address space             Virtual address space

0x000000000000                    0x000000000000
       |                                  |
       |                                  |
       v                                  v
  Page tables A                       Page tables B
       |                                  |
       v                                  v
  Physical memory                   Physical memory
```

The important point is that the virtual addresses used by two processes do not need to be the same.

For example:

```text
Process A:
    virtual address = 0x7f1234500000

Process B:
    virtual address = 0x7f9876500000
```

Yet both addresses can refer to the same physical page:

```text
Process A                         Process B

0x7f1234500000                    0x7f9876500000
        |                                  |
        v                                  v
   Page Table A                       Page Table B
        |                                  |
        |                                  |
        +---------------+------------------+
                        |
                        v
                  Physical Page X
```

This ability to share physical pages is used extensively by Linux.

---

## 2. Mapping the Same File in Two Processes

In a folder where we have [mapfile.c](mapfile.c), [check_pfn.c](check_pfn.c) and [Makefile](Makefile), run:

```bash
make
```

The `Makefile` automatically:

* Compiles `mapfile.c` into the `mapfile` executable.
* Compiles check_pfn.c into the check_pfn executable. We will use this program in a later step to determine the Page Frame Number (PFN) corresponding to a virtual address.
* Creates `data.bin` as a **4096-byte file (one page)**.
* Places recognizable text at the beginning of `data.bin` so that you can observe its contents when it is mapped.

You do not need to create or compile these files manually.

Now run two copies of the program in separate terminals:

```bash
./mapfile
```

and:

```bash
./mapfile
```

Each process will display its PID, the virtual address at which it mapped the file, and the contents of the mapped page. You should see something similar to:

```text
Process A:

test@test-vm:~/advanced_os/demos/shared_pages$ ./mapfile
PID: 3446
Virtual address: 0x7b88e944d000
Contents: Hello from the shared file!

Press ENTER to exit...

Process B:

test@test-vm:~/advanced_os/demos/shared_pages$ ./mapfile
PID: 3447
Virtual address: 0x737fc3c90000
Contents: Hello from the shared file!

Press ENTER to exit...
```

Notice that:

* the PIDs are different;
* the virtual addresses are different;
* both processes see the same file contents.

Keep both programs running. We will use them to investigate how **two processes can map the same file into their virtual address spaces**.

---

## 3. Examine the Process Address Spaces

While both programs are waiting at `getchar()`, open a third terminal, and inspect their memory mappings.

For example: (remember to replace 3446 with the PID of process A, and replace 3447 with the PID of process B.)

```bash
cat /proc/3446/maps
```


and:

```bash
cat /proc/3447/maps
```

Find the mapping corresponding to `data.bin`.

You should see something resembling:

```text
7b88e944d000-7b88e944e000 r--p 00000000 08:01 1502784                    /home/test/advanced_os/demos/shared_pages/data.bin
```

and in the other process:

```text
737fc3c90000-737fc3c91000 r--p 00000000 08:01 1502784                    /home/test/advanced_os/demos/shared_pages/data.bin
```

The exact addresses will differ between systems.

---

## 4. The Important Question

At this point we know:

```text
Process A:
    virtual address A
        ↓
    data.bin

Process B:
    virtual address B
        ↓
    data.bin
```

But we want to understand what happens at the physical-memory level. More specifically, are these two virtual addresses refer to the same physical page like this?

```text
Process A                         Process B

virtual address A                 virtual address B
       |                                 |
       v                                 v
   Page Table A                     Page Table B
       |                                 |
       +---------------+-----------------+
                       |
                       v
                 Physical Page X
```

Let's prove it. How can we determine whether those virtual addresses actually refer to the **same physical memory page**? Linux provides a kernel interface called `pagemap`:

```text
/proc/<pid>/pagemap
```

Each virtual page has a corresponding entry in this file. By parsing the corresponding entry for a virtual address, we can determine the physical page to which that virtual address is mapped. We will now use the program check_pfn, which was generated in Step 2 by the make command, to find the physical page corresponding to virtual address A and virtual address B. The program takes the PID and the virtual address as its command-line arguments. We open a third terminal and run the program like this:

```text
test@test-vm:~/advanced_os/demos/shared_pages$ sudo ./check_pfn 3446 0x7b88e944d000
PID:             3446
Virtual address: 0x7b88e944d000
PFN:             1670512
Physical address: 0x197d70000
```

and:

```text
test@test-vm:~/advanced_os/demos/shared_pages$ sudo ./check_pfn 3447 0x737fc3c90000
PID:             3447
Virtual address: 0x737fc3c90000
PFN:             1670512
Physical address: 0x197d70000
```

Notice that the virtual addresses are different, but the physical addresses are the same. The PFNs are also the same, confirming that both virtual addresses map to the same physical page frame.

This demonstrates that:

> **Two different processes can use different virtual addresses to access the same physical memory page.**

And this raises an important question: If the kernel has deliberately arranged for both processes to use the same physical page, what happens when Process A tries to write to that page? That leads directly to the Copy-on-write (COW) technique:

```text
             Physical Page Y
             /             \
        Process A       Process B
            |
          WRITE
            |
            v
       protection fault
            |
            v
        allocate Page Z
            |
            v
        copy Y → Z
            |
            v

Process A → Page Z
Process B → Page Y
```

## Follow-up Experiment

After completing the above experiment, we can proceed to [pagecache_probe.md](pagecache_probe.md) for a follow-up experiment where we investigate the important concept of page cache.

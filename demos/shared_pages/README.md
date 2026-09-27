# Demo: Shared Pages and Copy-on-Write

## Overview

In this demo, we will investigate how Linux allows multiple processes to map the same physical memory page, even though the processes have completely independent virtual address spaces.

---

# 1. Background

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

# 2. Mapping the Same File in Two Processes

In a folder where we have the [mapfile.c](mapfile.c), [check_pfn.c](check_pfn.c) and [Makefile](Makefile), run:

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
PID: 3291
Virtual address: 0x75df5adb1000
Contents: Hello from the shared file!

Press ENTER to exit...

Process B:

test@test-vm:~/advanced_os/demos/shared_pages$ ./mapfile
PID: 3292
Virtual address: 0x7794fb201000
Contents: Hello from the shared file!

Press ENTER to exit...
```

Notice that:

* the PIDs are different;
* the virtual addresses are different;
* both processes see the same file contents.

Keep both programs running. We will use them to investigate how **two processes can map the same file into their virtual address spaces**.

---

# 3. Examine the Process Address Spaces

While both programs are waiting at `getchar()`, open a third terminal, and inspect their memory mappings.

For example: (remember to replace 3291 with the PID of process A, and replace 3292 with the PID of process B.

```bash
cat /proc/3291/maps
```


and:

```bash
cat /proc/3292/maps
```

Find the mapping corresponding to `data.bin`.

You should see something resembling:

```text
75df5adb1000-75df5adb2000 r--p 00000000 08:01 1502844                    /home/test/advanced_os/demos/shared_pages/data.bin
```

and in the other process:

```text
7794fb201000-7794fb202000 r--p 00000000 08:01 1502844                    /home/test/advanced_os/demos/shared_pages/data.bin
```

The exact addresses will differ between systems.

---

# 4. The Important Question

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

Each virtual page has a corresponding entry in this file. The physical page is identified by a **Page Frame Number (PFN)**. In Step 2, the Makefile compiled the provided check_pfn.c source file into the check_pfn executable. We will now use that program to inspect the physical page corresponding to each virtual address printed by the two mapfile processes. To verify whether the two virtual addresses are mapped to the same physical page frame, we will run check_pfn once for each of the two mapfile processes and compare their PFNs.

For example:

```bash
sudo ./check_pfn 3291 0x75df5adb1000
```

and:

```bash
sudo ./check_pfn 3292 0x7794fb201000
```

Replace the PID and virtual address in these commands with the PID and virtual address reported by the mapfile program. The two commands should use the corresponding PID/address pair from each of the two running mapfile processes.

The program will report the PFN:

```text
PID:              3291
Virtual address:  0x7f1234567000
PFN:              123456
Physical address: 0x1e240000
```

and:

```text
PID:              3292
Virtual address:  0x7f9876543000
PFN:              123456
Physical address: 0x1e240000
```

Notice that the **virtual addresses are different**, but the **PFNs are the same**.

This demonstrates that:

> **Two different processes can use different virtual addresses to access the same physical memory page.**

This is an important foundation for understanding shared memory and Copy-on-Write.

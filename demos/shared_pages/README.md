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

In a folder where we have the [mapfile.c](mapfile.c) source file and [Makefile](Makefile), run:

```bash
make
```

The `Makefile` automatically:

* Compiles `mapfile.c` into the `mapfile` executable.
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

PID: 1000
Virtual address: 0x7f1234500000
Contents: Hello from the shared file!


Process B:

PID: 1001
Virtual address: 0x7f9876500000
Contents: Hello from the shared file!
```

Notice that:

* the PIDs are different;
* the virtual addresses are different;
* both processes see the same file contents.

Keep both programs running. We will use them to investigate how **two processes can map the same file into their virtual address spaces**.

---

# 3. Examine the Process Address Spaces

While both programs are waiting at `getchar()`, inspect their memory mappings.

For example:

```bash
cat /proc/1000/maps
```

and:

```bash
cat /proc/1001/maps
```

Find the mapping corresponding to `data.bin`.

You should see something resembling:

```text
7f1234500000-7f1234501000 r--p ... data.bin
```

and in the other process:

```text
7f9876500000-7f9876501000 r--p ... data.bin
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

But we want to understand what happens at the physical-memory level.

Conceptually:

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

Thus, two different virtual addresses can refer to one physical page.

---

# 5. Inspecting PFNs with `/proc/<pid>/pagemap`

Linux provides a mechanism called `pagemap` that can expose information about the physical page corresponding to a virtual address.

The relevant file is:

```text
/proc/<pid>/pagemap
```

The physical page is identified by a **Page Frame Number (PFN)**.

Conceptually:

```text
Virtual Address
       |
       v
    Page Table
       |
       v
      PFN
       |
       v
 Physical Page
```

So are virtual address A and virtual address B mapped to the same physical page frame?

```text
Process A:
    virtual address A → PFN X

Process B:
    virtual address B → PFN X
```

If both mappings produce the same PFN, then both virtual pages refer to the same physical page.

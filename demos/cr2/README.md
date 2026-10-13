CR2 Page Fault Verification Experiment

This repository contains an empirical systems experiment designed to prove a core operating system principle: CR2 precisely records the linear address associated with the most recent page fault on x86 architectures.

Overview & Background

On x86 processors, CR2 is a dedicated control register that holds the exact linear (virtual) address triggering a page fault exception (#PF). Because CR2 is a privileged register accessible only at Ring 0, user-space programs cannot read it directly.

To observe and verify CR2 empirically, this experiment combines:

A Linux Kernel Module using a kprobe on handle_mm_fault. When hit, it inspects CR2 via inline assembly and filters for our target process and address.

A User-Space Trigger Program that allocates memory via mmap at a fixed, page-aligned address (0x500000000000) and accesses an offset within it to deliberately induce a demand-paging fault.

Prerequisites & Project Structure

Your project directory should contain the following files:

cr2_probe.c — The kernel module interceptor.

trigger.c — The user-space test application.

Makefile — The build automation script.

How to Run the Experiment

Build the Binaries:
Compile both the kernel module and the user-space application using the provided Makefile:

make


Insert the Kernel Module:
Load the compiled probe into the running kernel:

sudo insmod cr2_probe.ko


Clear the Kernel Ring Buffer:
Wipe old dmesg entries to keep the log clean and focused:

sudo dmesg -C


Execute the Trigger Program:
Run your user-space test binary:

./trigger


Verify the Results:
Inspect the kernel log to confirm that CR2 successfully captured the exact faulting address:

sudo dmesg


Expected Output

User-Space Output

Successfully mapped base page at: 0x500000000000
Triggering page fault at offset address: 0x500000000041
Successfully read value: 0


Kernel Ring Buffer (sudo dmesg) Output

[ 4125.891234] CR2_PROBE: MATCH FOUND! Target address caught in CR2 = 0x500000000041


What This Proves

Hardware-Software Integration: When a page fault occurs, the CPU hardware automatically stores the faulting virtual address in the CR2 register before jumping to the page fault interrupt handler (#PF).

Kernel Observability: By placing a kprobe at handle_mm_fault and reading CR2 via inline assembly (mov %%cr2, %0), the kernel can inspect the exact address requested by user space.

Demand Paging Mechanics: The precise match between the user program's offset address (0x500000000041) and the captured CR2 register value empirically demonstrates how the CPU pinpoints unmapped or lazy-allocated memory references.

Cleanup

When finished, remove the kernel module and clean up the build files:

sudo rmmod cr2_probe
make clean


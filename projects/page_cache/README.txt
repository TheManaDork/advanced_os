vboxuser@AdvOpSysCourseVMDynamic:~/Documents/Github/advanced_os/projects/page_cache$ bash run.sh
========================================
Cleaning up previous module
========================================

========================================
Building the experiment
========================================
gcc -O0 -Wall -o readFile readFile.c
make -C /lib/modules/7.0.0-test/build M=/home/vboxuser/Documents/Github/advanced_os/projects/page_cache modules
make[1]: Entering directory '/home/vboxuser/Desktop/linux-7.0'
make[2]: Entering directory '/home/vboxuser/Documents/Github/advanced_os/projects/page_cache'
  CC [M]  pagecacheTest.o
  MODPOST Module.symvers
  CC [M]  pagecacheTest.mod.o
  CC [M]  .module-common.o
  LD [M]  pagecacheTest.ko
make[2]: Leaving directory '/home/vboxuser/Documents/Github/advanced_os/projects/page_cache'
make[1]: Leaving directory '/home/vboxuser/Desktop/linux-7.0'
dd if=/dev/urandom of=data.bin bs=4096 count=1
1+0 records in
1+0 records out
4096 bytes (4.1 kB, 4.0 KiB) copied, 8.2085e-05 s, 49.9 MB/s

========================================
Loading kernel module
========================================

========================================
Running readFile
========================================
readFile process started: PID 44720
PID: 44720
Read 4096 bytes
User buffer virtual address: 0x5677015c6000
First 16 bytes: 6a 5c 3f e1 ac c5 09 d8 8e 08 d3 c9 b9 9d 9b 2f
Waiting for kernel-module test...

========================================
Verifying readFile
========================================
    PID    PPID S COMMAND
  44720   44493 S readFile

readFile PID:     44720
User buffer:      0x5677015c6000

========================================
Running kernel-module test
========================================

========================================
Kernel output
========================================
[13497.758145] [PAGECACHE]: unloaded
[13585.525203] [PAGECACHE]: loaded
[13585.787914] === Cached Page: ========================================= 
[13585.787918] [PAGECACHE]: PID              = 44720
[13585.787919] [PAGECACHE]: Virtual Address  = 0x5677015c6000
[13585.787920] [PAGECACHE]: PFN       = 30483
[13585.787921] [PAGECACHE]: Physical Address = 0x7713000
[13585.787921] [PAGECACHE]: First 16 bytes:
[13585.787923] [PAGECACHE]: 6a
[13585.787968] [PAGECACHE]: 5c
[13585.787981] [PAGECACHE]: 3f
[13585.787982] [PAGECACHE]: e1
[13585.787982] [PAGECACHE]: ac
[13585.787983] [PAGECACHE]: c5
[13585.787983] [PAGECACHE]: 09
[13585.787984] [PAGECACHE]: d8
[13585.787984] [PAGECACHE]: 8e
[13585.787985] [PAGECACHE]: 08
[13585.787985] [PAGECACHE]: d3
[13585.787986] [PAGECACHE]: c9
[13585.787986] [PAGECACHE]: b9
[13585.787987] [PAGECACHE]: 9d
[13585.787987] [PAGECACHE]: 9b
[13585.787988] [PAGECACHE]: 2f
[13585.787993] === User Buffer: ========================================= 
[13585.788004] [PAGECACHE]: PID:               44720
[13585.788005] [PAGECACHE]: Virtual address:   0x5677015c6000
[13585.788006] [PAGECACHE]: PFN:               29786
[13585.788006] [PAGECACHE]: Physical address:  0x745a000
[13585.788007] [PAGECACHE]: First 16 bytes:
[13585.788008] [PAGECACHE]: 6a
[13585.788009] [PAGECACHE]: 5c
[13585.788009] [PAGECACHE]: 3f
[13585.788010] [PAGECACHE]: e1
[13585.788010] [PAGECACHE]: ac
[13585.788011] [PAGECACHE]: c5
[13585.788011] [PAGECACHE]: 09
[13585.788012] [PAGECACHE]: d8
[13585.788012] [PAGECACHE]: 8e
[13585.788013] [PAGECACHE]: 08
[13585.788013] [PAGECACHE]: d3
[13585.788014] [PAGECACHE]: c9
[13585.788014] [PAGECACHE]: b9
[13585.788014] [PAGECACHE]: 9d
[13585.788015] [PAGECACHE]: 9b
[13585.788015] [PAGECACHE]: 2f
[13585.788017] [PAGECACHE]: memcmp() = 0
[13585.788017] ========================================

========================================
Experiment complete
========================================

========================================
Cleaning up
========================================
Stopping readFile (PID 44720)...
Removing pagecacheTest...

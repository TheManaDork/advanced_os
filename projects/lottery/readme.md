My cpu code makes my entire vm freeze everytime and I haven't managed to figure out what wrong and I am well past out of time. I've been working on this for hours every day since it was due and budgeted before it was due as well.

Everything works except assigning processes to a cpu.


/lottery$ sudo bash run_lottery.sh
make -C /lib/modules/7.0.0-test/build M=/home/vboxuser/Documents/Github/advanced_os/projects/lottery modules
make[1]: Entering directory '/home/vboxuser/Desktop/linux-7.0'
make[2]: Entering directory '/home/vboxuser/Documents/Github/advanced_os/projects/lottery'
make[2]: Leaving directory '/home/vboxuser/Documents/Github/advanced_os/projects/lottery'
make[1]: Leaving directory '/home/vboxuser/Desktop/linux-7.0'
=========================================
 Installing 'lottery' Kernel Module
=========================================

=========================================
 Starting Multi-Core Experiment (20 Tasks)
 Target: lucas(47) across 20 processes
 Ticket Distribution: 10 to 200 tickets
=========================================

Spawning 20 processes...
  [Process 01] PID: 6672 | Tickets: 10
  [Process 02] PID: 6674 | Tickets: 10
  [Process 03] PID: 6676 | Tickets: 30
  [Process 04] PID: 6678 | Tickets: 30
  [Process 05] PID: 6680 | Tickets: 60
  [Process 06] PID: 6682 | Tickets: 60
  [Process 07] PID: 6684 | Tickets: 100
  [Process 08] PID: 6686 | Tickets: 150
  [Process 09] PID: 6688 | Tickets: 200

All 20 processes launched.
Waiting 3 seconds for initial process registration to settle...
--------------------------------------------------------
Capturing Steady-State Load Balancing Metrics...
--------------------------------------------------------
pid 6680, with 60 tickets: computing lucas(47) took 9.78 seconds.
pid 6676, with 30 tickets: computing lucas(47) took 15.03 seconds.
pid 6682, with 60 tickets: computing lucas(47) took 14.03 seconds.
pid 6688, with 200 tickets: computing lucas(47) took 16.28 seconds.
pid 6684, with 100 tickets: computing lucas(47) took 15.86 seconds.
pid 6686, with 150 tickets: computing lucas(47) took 17.53 seconds.
pid 6672, with 10 tickets: computing lucas(47) took 9.84 seconds.
pid 6678, with 30 tickets: computing lucas(47) took 8.65 seconds.
pid 6674, with 10 tickets: computing lucas(47) took 6.52 seconds.

=========================================
 Total Steady-State Execution Time: 18.706043552s
=========================================

=========================================
 Steady-State Queue Load Snapshots
=========================================
[ 2640.600599] [LOTTERY_STATUS]: Device registered at /dev/lottery
[ 2641.101480] [LOTTERY_STATUS] CPU0: 0 tix (0 tasks) | CPU1: 0 tix (0 tasks) |
[ 2641.114544] [LOTTERY_STATUS]: Process 6672 registered. index 0, weight 10
[ 2641.118914] [LOTTERY_STATUS]: Process 6674 registered. index 0, weight 10
[ 2641.122661] [LOTTERY_STATUS]: Process 6678 registered. index 1, weight 40
[ 2641.126640] [LOTTERY_STATUS]: Process 6676 registered. index 1, weight 40
[ 2641.129371] [LOTTERY_STATUS]: Process 6680 registered. index 2, weight 100
[ 2641.131584] [LOTTERY_STATUS]: Process 6682 registered. index 2, weight 100
[ 2641.133639] [LOTTERY_STATUS]: Process 6684 registered. index 3, weight 200
[ 2641.135814] [LOTTERY_STATUS]: Process 6688 registered. index 3, weight 300
[ 2641.136403] [LOTTERY_STATUS]: Process 6686 registered. index 4, weight 350
[ 2641.601629] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2642.102482] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2642.770349] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2643.106249] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2643.847118] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2644.844441] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2645.856800] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2646.100829] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2646.855298] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2647.848368] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2648.844303] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2649.843410] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2650.103464] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2650.605660] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2651.843187] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2652.705369] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2653.845309] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2654.105930] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 350 tix (5 tasks) |
[ 2654.606280] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 290 tix (4 tasks) |
[ 2655.845457] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 290 tix (4 tasks) |
[ 2656.108581] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 290 tix (4 tasks) |
[ 2656.609181] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 290 tix (4 tasks) |
[ 2657.107353] [LOTTERY_STATUS] CPU0: 300 tix (4 tasks) | CPU1: 290 tix (4 tasks) |
[ 2657.843333] [LOTTERY_STATUS] CPU0: 270 tix (3 tasks) | CPU1: 290 tix (4 tasks) |
[ 2658.105246] [LOTTERY_STATUS] CPU0: 10 tix (1 tasks) | CPU1: 190 tix (3 tasks) |
[ 2658.846771] [LOTTERY_STATUS] CPU0: 10 tix (1 tasks) | CPU1: 190 tix (3 tasks) |
[ 2659.104875] [LOTTERY_STATUS] CPU0: 10 tix (1 tasks) | CPU1: 190 tix (3 tasks) |
[ 2659.845226] [LOTTERY_STATUS] CPU0: 10 tix (1 tasks) | CPU1: 40 tix (2 tasks) |
[ 2660.107772] [LOTTERY_STATUS] CPU0: 10 tix (1 tasks) | CPU1: 40 tix (2 tasks) |
[ 2660.600851] [LOTTERY_STATUS] CPU0: 10 tix (1 tasks) | CPU1: 40 tix (2 tasks) |
[ 2661.844382] [LOTTERY_STATUS] CPU0: 10 tix (1 tasks) | CPU1: 40 tix (2 tasks) |
[ 2662.109908] [LOTTERY_STATUS] CPU0: 10 tix (1 tasks) | CPU1: 40 tix (2 tasks) |
[ 2662.605456] [LOTTERY_STATUS] CPU0: 10 tix (1 tasks) | CPU1: 30 tix (1 tasks) |
[ 2663.298470] [LOTTERY_STATUS] CPU0: 10 tix (1 tasks) | CPU1: 30 tix (1 tasks) |
[ 2663.843129] [LOTTERY_STATUS] CPU0: 10 tix (1 tasks) | CPU1: 0 tix (0 tasks) |
[ 2664.636554] [LOTTERY_STATUS] CPU0: 10 tix (1 tasks) | CPU1: 0 tix (0 tasks) |

=========================================
 Steady-State Migration Events
=========================================
No task migrations required during steady state (System in Equilibrium).

=========================================
 Cleaning Up Module
=========================================
Done.

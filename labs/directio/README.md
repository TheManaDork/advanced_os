# Lab 4 - Comparing Buffered I/O and Direct I/O

## Objective

In previous lectures, we discussed buffered I/O, memory-mapped I/O, and direct I/O. Buffered I/O uses the page cache to cache file data, while direct I/O bypasses the page cache for the requested I/O.

In this lab, you will use `fio` to compare the performance of buffered I/O and direct I/O on Linux.

You will demonstrate that:

- Buffered I/O can benefit from the Linux page cache.
- Repeated buffered reads can be faster than direct I/O because cached data can be served from memory.
- Direct I/O bypasses the page cache and performs storage I/O for the requested data.

## Step 1 — Install fio and jq

Install `fio`, a flexible I/O benchmarking tool, and `jq`, a command-line JSON processor.

On Ubuntu, run:

```bash
$ sudo apt update
$ sudo apt install fio jq
```

Verify that both programs are installed:

```bash
$ fio --version
$ jq --version
```

## Step 2 — Create a Test File

Create a 256 MiB test file named `testfile.bin` in your lab directory:

```bash
$ fio --name=setup \
    --filename=testfile.bin \
    --size=256M \
    --rw=write --bs=1M \
    --ioengine=sync --direct=0 \
    --output=/dev/null
```

Verify that the file exists:

```bash
$ ls -lh testfile.bin
```

The file should be approximately 256 MiB.

## Step 3 — Measure Buffered I/O Performance

First, read the entire file using buffered I/O:

```bash
$ fio --name=buffered \
    --filename=testfile.bin \
    --size=256M \
    --rw=read --bs=4k \
    --ioengine=sync --direct=0 \
    --output-format=json |
  jq -r '"Bandwidth: \((.jobs[0].read.bw / 1024) | floor) MiB/s\nTime: \(.jobs[0].read.runtime) ms"'
```

The command reports only the read bandwidth and the time required to read the file.

The output should look similar to:

```text
Bandwidth: 810 MiB/s
Time: 316 ms
```

Your results will vary depending on your machine and system load.

Run the same command a second time. Record the bandwidth and elapsed time again.

The second run may be faster because the file data may now be cached in the Linux page cache.

## Step 4 — Measure Direct I/O Performance

Now repeat the experiment using direct I/O by setting `--direct=1`:

```bash
$ fio --name=direct \
    --filename=testfile.bin \
    --size=256M \
    --rw=read --bs=4k \
    --ioengine=sync --direct=1 \
    --output-format=json |
  jq -r '"Bandwidth: \((.jobs[0].read.bw / 1024) | floor) MiB/s\nTime: \(.jobs[0].read.runtime) ms"'
```

The output should look similar to:

```text
Bandwidth: 304 MiB/s
Time: 843 ms
```

Record your results.

## Step 5 — Compare the Results

Create a table containing your measured results:

| I/O method | Bandwidth (MiB/s) | Time (ms) |
|---|---:|---:|
| Buffered I/O, first run | | |
| Buffered I/O, second run | | |
| Direct I/O | | |

## Step 6 — Clean Up

Remove the test file:

```bash
$ rm -f testfile.bin
```

## Submission

Submit screenshots on Submitty demonstrating your results. Also submit a txt or md file containing your completed comparison table.

Your screenshot(s) must clearly show:

- The commands used to benchmark buffered I/O and direct I/O. For buffered I/O, show the first run and second run - the second run should be much faster than the first run.
- The bandwidth and elapsed time reported by each command.

Due Date: 10/29/2026, 11:59pm. Each lab has a maximum of 5 points. Late submissions will be accepted within 3 days after the deadline, with a 1-point penalty applied to the earned grade.

fio --name=buffered \
    --filename=testfile.bin \
    --size=256M \
    --rw=read --bs=4k \
    --ioengine=sync --direct=0 \
    --output-format=json |
  jq -r '"Bandwidth: \((.jobs[0].read.bw / 1024) | floor) MiB/s\nTime: \(.jobs[0].read.runtime) ms"'
Bandwidth: 409 MiB/s
Time: 625 ms


***

fio --name=buffered     --filename=testfile.bin     --size=256M     --rw=read --bs=4k     --ioengine=sync --direct=0     --output-format=json |   jq -r '"Bandwidth: \((.jobs[0].read.bw / 1024) | floor) MiB/s\nTime: \(.jobs[0].read.runtime) ms"'
Bandwidth: 861 MiB/s
Time: 297 ms

***

fio --name=direct \
    --filename=testfile.bin \
    --size=256M \
    --rw=read --bs=4k \
    --ioengine=sync --direct=1 \
    --output-format=json |
  jq -r '"Bandwidth: \((.jobs[0].read.bw / 1024) | floor) MiB/s\nTime: \(.jobs[0].read.runtime) ms"'
Bandwidth: 6 MiB/s                              
Time: 42038 ms

***

fio --name=direct \
    --filename=testfile.bin \
    --size=256M \
    --rw=read --bs=4k \
    --ioengine=sync --direct=1 \
    --output-format=json |
  jq -r '"Bandwidth: \((.jobs[0].read.bw / 1024) | floor) MiB/s\nTime: \(.jobs[0].read.runtime) ms"'
Bandwidth: 3 MiB/s
Time: 65518 ms


| I/O method | Bandwidth (MiB/s) | Time (ms) |
|---|---:|---:|
| Buffered I/O, first run | 409 MiB/s | 625 ms |
| Buffered I/O, second run | 861 MiB/s| 297 ms |
| Direct I/O, first run | 6 MiB/s | 42038 ms |
| Direct I/O, first run | 3 MiB/s | 65518 ms |

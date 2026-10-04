# Memory Mapping Examples

These programs demonstrate Linux process memory, `mmap()`, page cache,
copy-on-write, and related concepts.

## Programs

### Basic file I/O

- `open_rd` - Open a file for reading.
- `open_rw` - Open a file for reading and writing.

### Basic `mmap()`

- `mmap_rd` - Map a file into memory for reading.
- `mmap_example` - Basic `MAP_SHARED` example. Modifications to the
  mapping are reflected in the underlying file.

### `MAP_SHARED` and `MAP_PRIVATE`

- `mmap_shared_rw` - Demonstrates a writable `MAP_SHARED` mapping.
  The underlying file must be opened with write permission.

- `mmap_private_rw` - Demonstrates a writable `MAP_PRIVATE` mapping.
  Writes use copy-on-write (COW), so the underlying file is not modified.

### Lazy memory allocation and page residency

- `mmap_lazy` - Maps a 256 MB file with `mmap()` and compares RSS
  before and after accessing a byte. Demonstrates that `mmap()` does
  not immediately load the entire file into physical memory.

- `read_into_buffer` - Allocates a 256 MB buffer and reads the entire
  file into it. Comparing its RSS with `mmap_lazy` demonstrates the
  difference between a virtual mapping and actually making the file
  contents resident in memory.

- `mmap_stream` - Maps a large file and accesses it in chunks while
  measuring RSS. Demonstrates that pages become resident as they are
  accessed. Also demonstrates `madvise(MADV_DONTNEED)` to allow
  resident pages to be discarded.

### Copy-on-write and security

- `mmap_private_madvise` - Demonstrates `MAP_PRIVATE`, copy-on-write,
  and `madvise()`.

- `dirty_cow` - Historical Dirty COW demonstration. Shows how a
  kernel vulnerability could violate the copy-on-write protection of
  a private file mapping. This example is intended for use in an
  isolated teaching environment and does not work on patched kernels.

## Building

Build all programs:

    make

Build a specific program:

    make mmap_lazy

Clean compiled programs:

    make clean

## Lazy `mmap()` demonstration

The `mmap_lazy` and `read_into_buffer` programs are intended to be
run together to compare two approaches to accessing a large file.

First create a 256 MB test file:

    dd if=/dev/zero of=large_file.bin bs=1M count=256

Run:

    ./mmap_lazy

The program maps the entire 256 MB file, but its RSS remains small
because the file is not loaded into memory all at once.

Then run:

    ./read_into_buffer

This program reads the entire 256 MB file into a memory buffer.
Its RSS should increase by approximately 256 MB.

The exact RSS values depend on the Linux kernel and system state.

## Notes

RSS (Resident Set Size) is the amount of memory belonging to the
process that is currently resident in physical memory.

A large virtual memory mapping does not imply that the corresponding
physical pages are all resident. With a file-backed `mmap()`, pages
are generally brought into memory when they are accessed.

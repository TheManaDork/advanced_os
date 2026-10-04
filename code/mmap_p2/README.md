# Memory Mapping Examples

These programs demonstrate how file access permissions interact with
`open()`, `mmap()`, `MAP_SHARED`, `MAP_PRIVATE`, copy-on-write (COW),
and `madvise()`.

Compile all programs with:

```bash
make
```

The programs should be run in the following order.

## 1. `open_rd.c`

```bash
./open_rd
```

**Expected: Succeeds**

The program opens `/etc/passwd` for reading only.

This demonstrates that the process is allowed to open the file when it
only requests read access.

---

## 2. `open_rw.c`

```bash
./open_rw
```

**Expected: Fails**

The program attempts to open `/etc/passwd` for both reading and writing.

The process does not have permission to open this file for writing, so
the `open()` operation fails.

---

## 3. `mmap_rd.c`

```bash
./mmap_rd
```

**Expected: Succeeds**

The program opens `/etc/passwd` for reading and creates a private,
read-only memory mapping.

The process can read the contents of the file through the memory
mapping.

---

## 4. `mmap_shared_rw.c`

```bash
./mmap_shared_rw
```

**Expected: Fails**

The program attempts to create a shared, read/write memory mapping of
`/etc/passwd`.

A shared writable mapping would allow writes through the mapping to
modify the underlying file. Therefore, the file descriptor must have
write permission.

Because `/etc/passwd` was opened for reading only, `mmap()` fails.

Importantly, the program does not even need to perform a write.
Requesting write permission for the shared mapping is enough for
`mmap()` to fail.

---

## 5. `mmap_private_rw.c`

```bash
./mmap_private_rw
```

**Expected: Succeeds**

The program creates a private, read/write memory mapping of
`/etc/passwd`, even though the file was opened for reading only.

This is allowed because `MAP_PRIVATE` means that writes through the
mapping do not modify the underlying file.

When the process writes to the mapping, copy-on-write (COW) creates a
private copy of the affected page. The process can then modify this
private copy.

After running the program, examine the actual `/etc/passwd` file.
The file contents will remain unchanged.

This demonstrates the important distinction between a writable private
mapping and a writable shared mapping.

---

## 6. `mmap_private_madvise.c`

```bash
./mmap_private_madvise
```

**Expected: Succeeds**

This program extends the previous example.

It first creates a writable private mapping and modifies the mapping.
The write creates a private COW copy of the affected page.

The program then uses `madvise()` with `MADV_DONTNEED` to tell the
kernel that the pages in the mapping are no longer needed.

The private modified page can then be discarded. When the program
accesses the mapping again, the original file-backed contents appear
again.

The important sequence is:

1. Create a writable private mapping.
2. Write to the mapping.
3. A private COW copy is created.
4. The private copy contains the modification.
5. Use `MADV_DONTNEED`.
6. The private copy can be discarded.
7. The original file-backed contents are seen again.

The actual `/etc/passwd` file is never modified.

---

## Summary

The examples demonstrate the following progression:

| Program | Operation | Expected |
|---|---|---|
| `open_rd.c` | Open `/etc/passwd` for reading | Succeeds |
| `open_rw.c` | Open `/etc/passwd` for reading and writing | Fails |
| `mmap_rd.c` | Read-only private mapping | Succeeds |
| `mmap_shared_rw.c` | Read/write shared mapping of a read-only file | Fails |
| `mmap_private_rw.c` | Read/write private mapping of a read-only file | Succeeds |
| `mmap_private_madvise.c` | Writable private mapping followed by `MADV_DONTNEED` | Succeeds |

The key distinction is:

**`MAP_SHARED` + write permission**

A write through the mapping can modify the underlying file, so the file
must be writable.

**`MAP_PRIVATE` + write permission**

A write through the mapping modifies a private COW copy instead of the
underlying file, so a read-only file can still be mapped with write
permission.

The final example shows that the private COW copy can subsequently be
discarded with `MADV_DONTNEED`, allowing the original file-backed
contents to appear again.

# System_Programming_ex1

Exercise 1 in the course Systems Programming — `my_cp`, a file copy program based on
system calls with a configurable buffer size (granularity) and internal timing.

## Files

| File | Description |
|---|---|
| `my_cp.c` | The source code of the program |
| `Makefile` | Builds the program with the required compilation flags |
| `my_cp` | The compiled executable (built on Linux) |
| `DESIGN.pdf` | The design document |
| `DESIGN.md` | The same design document in Markdown, for viewing on GitHub |

The design document is in this repository because the course website was closed for
uploads.

## Build

```
make
```

The Makefile runs exactly the compilation command required in the assignment:

```
gcc -Wall -Wextra -std=gnu11 -O2 my_cp.c -o my_cp
```

It compiles without errors and without warnings. `make clean` deletes the executable.

The executable in this repository was compiled on Ubuntu 24.04 (x86-64) with GCC 13.3.0.

## Usage

```
./my_cp <source_file> <destination_file> <granularity_bytes>
```

- `source_file` — the path of the file to copy from.
- `destination_file` — the path of the file to copy to. If it already exists it is
  overwritten, and if it does not exist it is created.
- `granularity_bytes` — a positive integer, the size in bytes of the buffer used for
  every `read` and `write` operation.

Example:

```
./my_cp input.txt output.txt 4096
```

The program prints the total time of the read and write operations in milliseconds.
The time of opening and closing the files is not included in the measurement.

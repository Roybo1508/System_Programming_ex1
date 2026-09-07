// this program copies a file using system calls (open, read, write, close) and not stdio functions
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[])
{
    int src_fd, dst_fd;
    int granularity;
    char *buffer;
    struct timespec start_time, end_time;
    long seconds, nanoseconds;
    double elapsed_ms;
    ssize_t bytes_read;

    // check that the number of arguments is exactly 4 (program name + 3 arguments)
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <source_file> <destination_file> <granularity_bytes>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // convert the granularity argument from a string to an int
    // atoi returns 0 for text that is not a number, so this also rejects invalid input
    granularity = atoi(argv[3]);
    if (granularity <= 0) {
        fprintf(stderr, "Error: granularity_bytes must be a positive integer (got \"%s\")\n", argv[3]);
        return EXIT_FAILURE;
    }

    // allocate the buffer on the heap in the size of the granularity, and check it succeeded
    buffer = malloc(granularity);
    if (buffer == NULL) {
        fprintf(stderr, "Error: failed to allocate a buffer of %d bytes\n", granularity);
        return EXIT_FAILURE;
    }

    // open the source file for reading only
    // open returns a file descriptor, a number that identifies the open file, or -1 if it failed
    src_fd = open(argv[1], O_RDONLY);
    if (src_fd == -1) {
        perror("open source file");
        free(buffer);
        return EXIT_FAILURE;
    }

    // open the destination file for writing, create it if it does not exist, empty it if it does
    // 0644 is the permissions of the file, and it is only used when the file is created
    dst_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dst_fd == -1) {
        perror("open destination file");
        // the source file is already open at this point, so close it and free the buffer before exiting
        close(src_fd);
        free(buffer);
        return EXIT_FAILURE;
    }

    // clock_gettime puts the current time into a timespec struct of seconds and nanoseconds
    // CLOCK_MONOTONIC is a clock that only goes forward, so it fits measuring how long something took
    // the start time is taken after both files are open, so only the copying itself is measured
    if (clock_gettime(CLOCK_MONOTONIC, &start_time) == -1) {
        perror("clock_gettime");
        close(src_fd);
        close(dst_fd);
        free(buffer);
        return EXIT_FAILURE;
    }

    while (1) {
        // read copies up to granularity bytes from the file into the buffer
        // it returns how many bytes it actually read, 0 at end of file, or -1 if it failed
        bytes_read = read(src_fd, buffer, granularity);

        if (bytes_read == -1) {
            // a signal interrupted the read before any data was read, so do the same read again
            if (errno == EINTR) {
                continue;
            }
            perror("read");
            close(src_fd);
            close(dst_fd);
            free(buffer);
            return EXIT_FAILURE;
        }

        // read returned 0 which means end of file, so stop the loop
        if (bytes_read == 0) {
            break;
        }

        // write returns how many bytes it actually wrote, and it can be less than we asked for
        // so we keep writing until the whole chunk was written, each time from the first byte not written yet
        ssize_t total_written = 0;
        while (total_written < bytes_read) {
            ssize_t bytes_written = write(dst_fd, buffer + total_written, bytes_read - total_written);

            if (bytes_written == -1) {
                // a signal interrupted the write, so try again from the same place in the buffer
                if (errno == EINTR) {
                    continue;
                }
                perror("write");
                close(src_fd);
                close(dst_fd);
                free(buffer);
                return EXIT_FAILURE;
            }

            total_written += bytes_written;
        }
    }

    // take the end time right after the loop finished, before closing the files
    if (clock_gettime(CLOCK_MONOTONIC, &end_time) == -1) {
        perror("clock_gettime");
        close(src_fd);
        close(dst_fd);
        free(buffer);
        return EXIT_FAILURE;
    }

    // close releases the file descriptors back to the operating system, and returns -1 if it failed
    if (close(src_fd) == -1) {
        perror("close source file");
    }
    if (close(dst_fd) == -1) {
        perror("close destination file");
    }
    free(buffer);

    seconds = end_time.tv_sec - start_time.tv_sec;
    nanoseconds = end_time.tv_nsec - start_time.tv_nsec;
    // if the nanoseconds came out negative, borrow one second and add 1000000000 nanoseconds
    if (nanoseconds < 0) {
        seconds = seconds - 1;
        nanoseconds = nanoseconds + 1000000000L;
    }

    // convert the seconds and nanoseconds into one number in milliseconds
    elapsed_ms = seconds * 1000.0 + nanoseconds / 1000000.0;
    printf("Copy finished in %.3f milliseconds (granularity: %d bytes)\n", elapsed_ms, granularity);

    return EXIT_SUCCESS;
}

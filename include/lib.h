#ifndef LIB_H
#define LIB_H

#include "linux.h"
#include "x64.h"
#include "types.h"

static __attribute__((unused))
long read(unsigned int fd, const void *buf, size_t count) {
	return syscall3(_NR_READ, fd, buf, count);
}

static __attribute__((unused))
long write(unsigned int fd, const void *str, size_t count) {
	return syscall3(_NR_WRITE, fd, str, count);
}

static __attribute__((unused))
int openat_r(int dirfd, const char *path, int flags, unsigned int mode) {
	return syscall4(_NR_OPENAT, dirfd, path, flags, mode);
}

static __attribute__((unused))
int openat(int dirfd, const char *path, int flags, ...) {
	unsigned int mode = 0;

	if (flags & O_CREAT) {
		va_list args;

		va_start(args, flags);
		mode = va_arg(args, unsigned int);
		va_end(args);
	}

	return openat_r(dirfd, path, flags, mode);
}

static __attribute__((unused))
int open_r(const char *path, int flags, unsigned int mode) {
	return syscall4(_NR_OPENAT, AT_FDCWD, path, flags, mode);
}

static __attribute__((unused))
int open(const char *path, int flags, ...) {
	unsigned int mode = 0;

	if (flags & O_CREAT) {
		va_list args;

		va_start(args, flags);
		mode = va_arg(args, unsigned int);
		va_end(args);
	}

	return open_r(path, flags, mode);
}

static __attribute__((unused))
int close(int fd) {
	return syscall1(_NR_CLOSE, fd);
}

static __attribute__((noreturn))
void exit(int status) {
	syscall1(_NR_EXIT, status & 255);
	while (1);
}

#endif

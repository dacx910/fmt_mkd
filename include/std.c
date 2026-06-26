#include "std.h"

const char errstr[] = "(error)";
const char nulstr[] = "(nil)";
const char hex[16] = "0123456789abcdef";

size_t strlen(const char *str) {
	const char *cur = str;

	while (*cur) {
		++cur;
	}
	return cur-str;
}

int print(unsigned int fd, const char *str) {
	return write(fd, str, strlen(str));
}

int puts(const char *str) {
	return print(stdout, str);
}

int error(const char *str) {
	return print(stderr, str);
}

char itoa_buf[22];

int utoa_w(unsigned long num, char *buf) {
	unsigned long lim;
	int digits = 0;
	int pos = 19;
	int dig;
	
	do {
		for (dig = 0, lim = 1; dig < pos; ++dig)
			lim *= 10;
		
		if (digits || num >= lim || !pos) {
			for (dig = 0; num >= lim; ++dig)
				num -= lim;
			buf[digits++] = '0' + dig;
		}
	} while (pos--);
	buf[digits] = '\0';
	return digits;
}

__inline__
char * utoa(unsigned long num) {
	utoa_w(num, itoa_buf);
	return itoa_buf;
}

int itoa_r(long num, char *buf) {
	char *ptr = buf;
	int len = 0;

	if (num < 0) {
		num = -(unsigned long)num;
		*(ptr++) = '-';
		len++;
	}
	len += utoa_w(num, ptr);
	return len;
}

__inline__
char * itoa(long num) {
	itoa_r(num, itoa_buf);
	return itoa_buf;
}

int itoh_r(long num, char *buf) {
	long i, curByte, digits = 2;
	int seenNonZero = 0;

	for (i = 15; i >= 0; --i) {
		curByte = (num >> (i*4)) & 0xf;
		if (seenNonZero) {
			buf[15-i] = hex[curByte];
			++digits;
		} else if (curByte) {
			buf[15-i] = hex[curByte];
			++digits;
			seenNonZero = 1;
		}
	}

	return digits;
}

char * itoh(long num) {
	itoh_r(num, itoa_buf);
	return itoa_buf;
}

char * itop(long num) {
	itoa_buf[0] = '0';
	itoa_buf[1] = 'x';
	itoh_r(num, itoa_buf+2);
	return itoa_buf;
}

__attribute__((format(printf, 2, 0)))
int fprintf_w(unsigned int fd, const char *fstr, va_list args) {
	unsigned long v;
	char * strarg;
	int matches = 0;
	const char *cur = fstr;
	const char *last = fstr;
	
	while (*cur) {
		if (*cur == '%') {
			++cur;
			switch (*cur) {
				case 'd':
					v = va_arg(args, long);
					write(fd, last, cur-last-1);
					print(fd, itoa(v));
					++cur;
					last = cur;
					++matches;
					break;
				case 's':
					strarg = va_arg(args, char *);
					write(fd, last, cur-last-1);
					print(fd, strarg);
					++cur;
					last = cur;
					++matches;
					break;
				case 'x':
					v = va_arg(args, long);
					write(fd, last, cur-last-1);
					print(fd, itoh(v));
					++cur;
					last = cur;
					++matches;
					break;
				case 'p':
					v = va_arg(args, long);
					write(fd, last, cur-last-1);
					print(fd, itop(v));
					++cur;
					last = cur;
					++matches;
				case '%':
					++cur;
					break;
				case 0:
					if (cur != last)
						write(fd, last, cur-last);
					return 0;
				default:
					write(fd, errstr, sizeof(errstr)-1);
					return -1;
			}
		} else {
			++cur;
		}
	}
	if (cur != last)
		write(fd, last, cur-last);

	return matches;
}

__attribute__((format(printf, 2, 3)))
int fprintf(unsigned int fd, const char *fstr, ...) {
	va_list args;
	int ret;

	va_start(args, fstr);
	ret = fprintf_w(fd, fstr, args);
	va_end(args);
	return ret;
}

__attribute__((format(printf, 1, 2)))
int printf(const char *fstr, ...) {
	va_list args;
	int ret;

	va_start(args, fstr);
	ret = fprintf_w(stdout, fstr, args);
	va_end(args);
	return ret;
}

char * strerror(int num) {
	if (num < 0)
		num = -num;
	switch (num) {
		case 1:
			return "Operation not permitted";
		case 2:
			return "No such file or directory";
		case 3:
			return "No such process";
		case 4:
			return "Interrupted system call";
		case 5:
			return "I/O error";
		case 6:
			return "No such device or address";
		case 7:
			return "Argument list too long";
		case 8:
			return "Exec format error";
		case 9:
			return "Bad file number";
		case 10:
			return "No child processes";
		case 11:
			return "Try again";
		case 12:
			return "Out of memory";
		case 13:
			return "Permission denied";
		case 14:
			return "Bad address";
		case 15:
			return "Block device required";
		case 16:
			return "Device or resource busy";
		case 17:
			return "File exists";
		case 18:
			return "Cross-device link";
		case 19:
			return "No such device";
		case 20:
			return "Not a directory";
		case 21:
			return "Is a directory";
		case 22:
			return "Invalid argument";
		case 23:
			return "File table overflow";
		case 24:
			return "Too many open files";
		case 25:
			return "Not a typewriter";
		case 26:
			return "Text file busy";
		case 27:
			return "File too large";
		case 28:
			return "No space left on device";
		case 29:
			return "Illegal seek";
		case 30:
			return "Read-only file system";
		case 31:
			return "Too many links";
		case 32:
			return "Broken pipe";
		case 33:
			return "Math argument out of domain of func";
		case 34:
			return "Math result not representable";
		default:
			return utoa(num);
	}
}
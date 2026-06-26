#ifndef LINUX_H
#define LINUX_H

#define stdin 0
#define stdout 1
#define stderr 2

#define _NR_READ 0
#define _NR_WRITE 1
#define _NR_OPEN 2
#define _NR_CLOSE 3
#define _NR_EXIT 60
#define _NR_OPENAT 257

// File flags
#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR 2
#define O_CREAT (1 << 6)
#define O_DIRECTORY (1 << 16)
#define O_EXCL (1 << 7)
#define O_NOCTTY (1 << 8)
#define O_NOFOLLOW (1 << 17)
#define O_TMPFILE (1 << 22) | O_DIRECTORY
#define O_TRUNC (1 << 9)
#define O_CLOEXEC (1 << 19)
#define O_APPEND (1 << 10)
#define O_NONBLOCK (1 << 11)
#define O_PATH (1 << 21)
#define O_EMPTYPATH (1 << 26)
#define O_NOATIME (1 << 18)
#define O_DIRECT (1 << 14)


// File modes
#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

#define S_ISUID 0004000
#define S_ISGID 0002000
#define S_ISVTX 0001000

#define AT_FDCWD -100

// Errors
#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */


#endif

#ifndef CRT_H
#define CRT_H

char **environ __attribute__((weak));

void _start(void);
static void exit(int);

__attribute__((weak))
void _start_c(long *sp) {
	long argc;
	char **argv;
	char **envp;
	int exitcode;

	int _nolibc_main(int, char **, char **) __asm__ ("main");

	argc = *sp;
	argv = (char **)(sp + 1);
	environ = envp = argv + argc + 1;

	exitcode = _nolibc_main(argc, argv, envp);

	exit(exitcode);
}

#endif

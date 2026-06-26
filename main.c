#include "include/std.h"
#include "include/string.h"

#define BUF_SIZE 0x400

int parse(int mdFile) {
	char buf[BUF_SIZE], *codeBlockUpper, *codeBlockLower, *cur;
	int open = 0;
	size_t nBytes = 0;

	while (nBytes = read(mdFile, buf, BUF_SIZE), nBytes > 0) {
		cur = buf;
		do {
			codeBlockUpper = strstr(cur, "```");

			if (codeBlockUpper) {
				if (open) {
					if (write(stdout, buf, (codeBlockUpper+4)-buf) < 0)
						fprintf(stderr, "ERROR: buf = %d, cBU = %d\n", buf, codeBlockUpper);
					open = 0;
					cur = codeBlockUpper+4;
					continue;
				}

				codeBlockLower = strstr(codeBlockUpper+3, "```");
				if (codeBlockLower) {
					if (write(stdout, codeBlockUpper, (codeBlockLower+4)-codeBlockUpper) < 0)
						fprintf(stderr, "ERROR: cBU = %d, cBL = %d\n", codeBlockUpper, codeBlockLower);
					cur = codeBlockLower+4;
				} else {
					if (write(stdout, codeBlockUpper, nBytes-(codeBlockUpper-buf)) < 0)
						fprintf(stderr, "ERROR: cBU = %d, nBytes = %d, buf = %d\n", codeBlockUpper, nBytes, buf);
					open = 1;
					break;
				}
			}
		} while (codeBlockUpper && nBytes == BUF_SIZE);

		if (nBytes < BUF_SIZE)
			break; // Avoid extra syscall
	}
	if (nBytes < 0) {
		fprintf(stderr, "Reading file failed: %s\n", strerror(nBytes));
		return -1;
	}

	return 0;
}

void usage(char *progName) {
	printf("Usage: %s [OPTION]... [FILE]\n", progName);
	puts("Removes code blocks and Hugo preludes, then performs a wordcount and analysis.\n"
		 "Example: fmt_mkd post.md\n\n"
		
		 "Options:\n"
		 "  -h\tDisplay help menu\n");
}

int main(int argc, char *argv[], char *envp[]) {
	char *fileName = 0;
	int retCode = 0, res = 0, mdFile = -1;

	if (argc < 2) {
		usage(argv[0]);
		retCode = -1;
		goto exit;
	}
	if (argc >= 3) {
		// Do arg parsing logic
	}

	fileName = argv[argc-1];
	mdFile = open(fileName, O_RDONLY, 0);
	if (mdFile < 0) {
		fprintf(stderr, "Could not open markdown file '%s': %s\n", fileName, strerror(mdFile));
		retCode = -2;
		goto exit;
	}

	if (parse(mdFile))
		goto exit;

exit:
	if (mdFile >= 0) {
		res = close(mdFile);
		if (res < 0) {
			fprintf(stderr, "Could not close fd: %s\n", strerror(res));
			retCode = -3;
		}
	}
	return retCode;
}

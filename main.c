#include "include/std.h"
#include "include/string.h"

#define BUF_SIZE 0x400

int parse(int mdFile) {
	char buf[BUF_SIZE], *upper, *lower;
	int foundPrelude = 0, openCodeBlock = 0;
	size_t nBytes = 0;

	while (nBytes = read(mdFile, buf, BUF_SIZE-1), nBytes > 0) {
		buf[nBytes] = '\0';
		if (foundPrelude == 2) {
			upper = strstr(buf, "```");
			if (upper) {
				if (openCodeBlock) {
					openCodeBlock = 0;
					write(stdout, upper+4, nBytes-(upper-buf+4));
				} else {
					if (upper > buf)
						write(stdout, buf, upper-buf);
					lower = strstr(buf+3, "```");
					if (lower)
						write(stdout, lower+4, nBytes-(lower+4-upper));
					else
						openCodeBlock = 1;
				}
			} else if (!openCodeBlock) {
				write(stdout, buf, nBytes);
			}
		} else if (foundPrelude == 1) {
			lower = strstr(buf, "---");
			if (lower) {
				foundPrelude = 2;
				write(stdout, lower+4, nBytes-(lower-buf+4));
			}
		} else {
			upper = strstr(buf, "---");
			if (upper) {
				if (upper+3)
				lower = strstr(upper+3, "---"); // OOB read when upper is the last bit of a string.
				if (upper != buf)
					error("[WARN]: First 3 bytes was not '---', ignoring content before prelude.\n");
				if (lower) {
					write(stdout, lower+4, nBytes-(lower-upper+4));
					foundPrelude = 2;
				} else {
					foundPrelude = 1;
				}
			} else {
				error("[WARN]: Could not find prelude within first buffer, skipping.\n");
				write(stdout, buf, nBytes);
				foundPrelude = 2;
			}
		}
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

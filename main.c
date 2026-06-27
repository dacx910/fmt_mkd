#include "include/std.h"
#include "include/string.h"

#define BUF_SIZE 0x400

int parse(int mdFile) {
	char buf[BUF_SIZE], *upper, *lower, *searchPos;
	int foundPrelude = 0, openCodeBlock = 0;
	size_t nBytes = 0;

	while (nBytes = read(mdFile, buf, BUF_SIZE-1), nBytes > 0) {
		buf[nBytes] = '\0';
		searchPos = buf;
		if (foundPrelude == 2) {
			do {
code_parse_loop:
				upper = strstr(searchPos, "```");
				if (upper) { // searchPos contains some code
					if (openCodeBlock) {
						searchPos = upper+4;
						openCodeBlock = 0;
					} else {
						if (searchPos < upper) {
							write(stdout, searchPos, upper-searchPos);
						}
						lower = strstr(upper+3, "```");
						if (lower) {
							searchPos = lower+4;
						} else {
							openCodeBlock = 1;
							break;
						}
					}
				} else if (openCodeBlock) { // searchPos must contain all code
					break;
				} else { // searchPos must contain all valid text
					write(stdout, searchPos, nBytes-(searchPos-buf));
					break;
				}
			} while (searchPos < buf+nBytes);
		} else if (foundPrelude == 1) {
			lower = strstr(buf, "---");
			if (lower) {
				foundPrelude = 2;
				searchPos = lower+4;
				goto code_parse_loop;
			}
		} else {
			upper = strstr(buf, "---");
			if (upper) {
				lower = strstr(upper+3, "---");
				if (upper != buf)
					error("[WARN]: First 3 bytes was not '---', ignoring content before prelude.\n");
				if (lower) {
					foundPrelude = 2;
					searchPos = lower+4;
					goto code_parse_loop;
				} else {
					foundPrelude = 1;
				}
			} else {
				error("[WARN]: Could not find prelude within first buffer, skipping.\n");
				foundPrelude = 2;
				goto code_parse_loop;
			}
		}
		if (nBytes < BUF_SIZE-1)
			break; // Avoid extra syscall
	}
	if (nBytes < 0) {
		fprintf(stderr, "Reading file failed: %s\n", strerror(nBytes));
		return -1;
	}

	return 0;
}

void usage(char *progName) {
	fprintf(stderr, "Usage: %s [OPTION]... [FILE]\n", progName);
	print(stderr, "Removes code blocks and Hugo preludes, then performs a wordcount and analysis.\n"
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

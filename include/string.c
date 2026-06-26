#include "string.h"

char * strchr(char *str, char tok) {
	char *cur = str;
	while (*cur) {
		if (*cur == tok)
			return cur;
		else
		 ++cur;
	}
	return 0;
}

char * strrchr(char *str, char tok) {
	char *cur = str + strlen(str) - 1;

	while (cur >= str) {
		if (*cur == tok)
			return cur;
		else
		 --cur;
	}
	return 0;
}

char * strstr(char *haystack, char *needle) {
	char *cur = haystack;
	size_t needleLen = strlen(needle), i;

	while (*cur) {
		if (*cur == needle[0]) {
			for (i = 1; i < needleLen; ++i) {
				if (cur[i] != needle[i])
					goto strstr_leave;
			}
			return cur;
		}
strstr_leave:
		++cur;
	}

	return 0;
}
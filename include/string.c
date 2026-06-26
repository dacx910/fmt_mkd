#include "string.h"

char * strchr(const char *str, const char tok) {
	const char *cur = str;
	while (*cur) {
		if (*cur == tok)
			return cur;
		else
		 ++cur;
	}
	return 0;
}

char * strrchr(const char *str, const char tok) {
	const char *cur = str + strlen(str) - 1;

	while (cur >= str) {
		if (*cur == tok)
			return cur;
		else
		 --cur;
	}
	return 0;
}

char * strstr(const char *haystack, const char *needle) {
	const char *cur = haystack;
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

int strcmp(const char *s1, const char *s2) {
	size_t s1_len = strlen(s1);
	size_t s2_len = strlen(s2);

	if (s1_len != s2_len)
		return -1;
	return memcmp(s1, s2, s1_len);
}
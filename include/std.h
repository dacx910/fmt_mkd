#ifndef STD_H
#define STD_H

#include "lib.h"

size_t strlen(const char *);

int print(unsigned int, const char *);
int puts(const char *);
int error(const char *);

char * utoa(unsigned long);
char * itoa(long);
char * itop(long);
char * itoh(long);

int fprintf(unsigned int, const char *, ...);
int printf(const char *, ...);

char * strerror(int);

int memcmp(const void *, const void *, size_t);

#endif
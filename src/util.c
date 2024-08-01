#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "util.h"

void die(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(1);
}

void *ecalloc(size_t nmemb, size_t size)
{
	void *p;
	if (!(p = calloc(nmemb, size)))
		die("calloc:");
	return p;
}

uint64_t rand_sparse_u64(void) { return rand_u64() & rand_u64() & rand_u64(); }

/* xorshiro */
uint64_t rand_u64(void)
{
	static struct {
		uint64_t a;
		uint64_t b;
	} state = {0x37B907B71847EECBULL, 0x9BE14C52E18EA764ULL};

	uint64_t a = state.a, b = state.b;
	uint64_t res = a + b;
	b ^= a;
	state.a = ((a << 55) | (a >> 9)) ^ b ^ (b << 14);
	state.b = (b << 36) | (b >> 28);
	return res;
}

uint64_t gettime(void)
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
	return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

#ifndef KNUR_UTIL_H_
#define KNUR_UTIL_H_

#include <stddef.h>
#include <stdint.h>

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#define ARRAY_FILL(arr, val)                                                   \
	do {                                                                   \
		for (unsigned __i = 0; __i < ARRAY_SIZE(arr); __i++)           \
			arr[__i] = val;                                        \
	} while (0)

[[noreturn]] void die(const char *fmt, ...);
void *ecalloc(size_t nmemb, size_t size);
uint64_t rand_sparse_u64(void);
uint64_t rand_u64(void);
uint64_t gettime(void);

#endif /* KNUR_UTIL_H_ */

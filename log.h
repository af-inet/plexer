#ifndef PLXR_LOG_H
#define PLXR_LOG_H

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define WARN(msg) \
	fprintf(stdout, "[%s:%d] " msg "\n", __FILE__, __LINE__)

#define WARNF(fmt, ...) \
	fprintf(stdout, "[%s:%d] " fmt "\n", __FILE__, __LINE__, __VA_ARGS__)

/* Should be called immediately after a syscall fails and sets `errno` (ideally on the very next line),
 * this way we can use `errno` knowing it hasn't been overwritten by another function.
 */
#define ERROR(name) \
	fprintf(stdout, "[%s:%d] %s: %s" "\n", __FILE__, __LINE__, name, strerror(errno))

#endif /* PLXR_LOG_H */

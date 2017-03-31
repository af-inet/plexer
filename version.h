#ifndef PLXR_VERSION_H
#define PLXR_VERSION_H

/* Currently versioning isn't very important, but it will be nice to know which binary you're using.
 */

#define xstr(a) str(a)
#define str(a) #a

#define PLXR_VERSION_MAJOR 1
#define PLXR_VERSION_MINOR 0
#define PLXR_VERSION_PATCH 0

#ifndef PLXR_COMMIT_HASH
#warning "Missing git commit hash, compiling with incomplete version string."
#define PLXR_COMMIT_STRING ""
#else
#define PLXR_COMMIT_STRING " (" xstr(PLXR_COMMIT_HASH) ")"
#endif

#define CONCAT(a,b,c,d) \
	xstr(a) "." xstr(b) "." xstr(c) d

#define PLXR_VERSION_STRING CONCAT(PLXR_VERSION_MAJOR, PLXR_VERSION_MINOR, PLXR_VERSION_PATCH, PLXR_COMMIT_STRING)

#endif /* PLXR_VERSION_H */

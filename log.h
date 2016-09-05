#ifndef PLXR_LOG_H
#define PLXR_LOG_H

#define WARNING(msg) \
printf("plxr WARNING: %s:%d %s", __FILE__, __LINE__, msg "\n") \

#define ERROR(msg) \
printf("plxr ERROR: %s:%d %s", __FILE__, __LINE__, msg "\n"); \
perror(msg) \

#define FATAL(msg) \
printf("plxr FATAL: %s:%d %s", __FILE__, __LINE__, msg "\n"); \
perror(msg); \
exit(EXIT_FAILURE) \

#endif /* PLXR_LOG_H */

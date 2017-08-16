#ifndef BACKTRACE_H
#define BACKTRACE_H
#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
typedef int (*backtrace_callback)(uintptr_t ip, void* args);

enum {
    BACKTRACE_CONTINUE,
    BACKTRACE_ABORT,
};

int mybacktrace(backtrace_callback callback, void* args, void* uc_mcontext);
#ifdef __cplusplus
}
#endif // __cplusplus
#endif /* BACKTRACE_H */


// SPDX-License-Identifier: MIT

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if __STDC_VERSION__ < 201112L // C11
#error "Requires at least C11!"
#elif __STDC_VERSION__ < 202311L // C23
#define static_assert _Static_assert
#endif

#ifndef __FILE_NAME__
#define __FILE_NAME__ (__builtin_strrchr(__FILE__, '/') + 1)
#endif



#ifdef BADGEROS_KERNEL
#include "log.h"
#include "meta.h"

// Assert based on a condition in both debug and release builds.
#define assert_always(condition)                                                                                       \
    do {                                                                                                               \
        if (__builtin_expect((condition) == 0, 0)) {                                                                   \
            logkf(LOG_FATAL, "%{cs}:%{d}: Assertion %{cs} failed.", __FILE_NAME__, __LINE__, #condition);              \
            __builtin_trap();                                                                                          \
        }                                                                                                              \
    } while (false)

#else
#include "stdio.h"

// Assert based on a condition in both debug and release builds.
#define assert_always(condition)                                                                                       \
    do {                                                                                                               \
        if (__builtin_expect((condition) == 0, 0)) {                                                                   \
            fflush(stdout);                                                                                            \
            fprintf(stderr, "\033[31m%s:%d: Assertion %s failed.\033[0m", __FILE_NAME__, __LINE__, #condition);        \
            fflush(stderr);                                                                                            \
            __builtin_trap();                                                                                          \
        }                                                                                                              \
    } while (false)

#endif


#ifdef NDEBUG
// Assert based on a condition in debug builds, run normally in release builds.
#define assert_dev_keep(condition) ((void)(condition))
// Assert based on a condition in debug builds.
#define assert_dev_drop(condition) ((void)0)
#else
// Assert based on a condition in debug builds, run normally in release builds.
#define assert_dev_keep(condition) assert_always(condition)
// Assert based on a condition in debug builds.
#define assert_dev_drop(condition) assert_always(condition)
#endif

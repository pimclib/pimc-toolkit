#pragma once

/*!
 * If used as a condition in the ``if`` statement causes the code that follows
 * to be the primary branch.
 */
#ifdef __GNUC__
#define PIMC_LIKELY(x)   __builtin_expect(!!(x),1)
#else
#define PIMC_LIKELY(x)   x
#endif // #ifdef __GNUC__

/*!
 * If used as a condition in the ``if`` statement causes the code that follows
 * to be the secondary branch.
 */
#ifdef __GNUC__
#define PIMC_UNLIKELY(x) __builtin_expect(!!(x),0)
#else
#define PIMC_UNLIKELY(x) x
#endif // #ifdef __GNUC__

/*!
 * Causes the function to be never inlined.
 */
#define PIMC_NO_INLINE    __attribute__((noinline))

/*!
 * Designates the hot path.
 */
#define PIMC_HOT_PATH     __attribute__((hot))

/*!
 * Designates the cold path
 */
#define PIMC_COLD_PATH    __attribute__((cold))

#if defined(__clang__) || (__GNUC__ >= 8)
#define PIMC_FALLTHROUGH [[fallthrough]];
#elif defined(__GNUC__)
#define FALLTHROUGH [[gnu::fallthrough]]
#else
#define FALLTHROUGH
#endif

/*!
 * Causes the code to be always inlined. This feature can be turned off
 * by defining ``PIMC_DISABLE_ALWAYS_INLINE``.
 */
#ifndef PIMC_DISABLE_ALWAYS_INLINE
#define PIMC_ALWAYS_INLINE __attribute__((always_inline)) inline
#else
#define PIMC_ALWAYS_INLINE inline
#endif

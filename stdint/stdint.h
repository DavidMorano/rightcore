/* stdint.h */
 
 
/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This is a total hack. I tried to find out what is supposed to be in here
        and made up guesses (based on standards printed on the web).

	Mostly: if it is not defined below, it is probably already defined in
	existing system headers.


*******************************************************************************/


#ifndef _STDINT_H_
#define _STDINT_H_	1


#include	<sys/int_types.h>

#if !defined(_XOPEN_SOURCE) || defined(__EXTENSIONS__)
#include	<sys/int_limits.h>
#include	<sys/int_const.h>
#endif


/* the new required type-defs */

typedef int8_t		int_least8_t ;
typedef int16_t		int_least16_t ;
typedef int32_t		int_least32_t ;
typedef int64_t		int_least64_t ;

typedef uint8_t		uint_least8_t ;
typedef uint16_t	uint_least16_t ;
typedef uint32_t	uint_least32_t ;
typedef uint64_t	uint_least64_t ;

typedef int8_t		int_fast8_t ;
typedef int16_t		int_fast16_t ;
typedef int32_t		int_fast32_t ;
typedef int64_t		int_fast64_t ;

typedef uint8_t		uint_fast8_t ;
typedef uint16_t	uint_fast16_t ;
typedef uint32_t	uint_fast32_t ;
typedef uint64_t	uint_fast64_t ;


/* these are the new defines for the new type-defs */

#if (! defined(__cplusplus)) || defined(__STDC_LIMIT_MACROS)

#ifndef	INT_LEAST8_MIN
#define INT_LEAST8_MIN    INT8_MIN
#define INT_LEAST16_MIN   INT16_MIN
#define INT_LEAST32_MIN   INT32_MIN
#define INT_LEAST64_MIN   INT64_MIN
#endif

#ifndef	INT_LEAST8_MAX
#define INT_LEAST8_MAX    INT8_MAX
#define INT_LEAST16_MAX   INT16_MAX
#define INT_LEAST32_MAX   INT32_MAX
#define INT_LEAST64_MAX   INT64_MAX
#endif

#ifndef	UINT_LEAST8_MAX
#define UINT_LEAST8_MAX   UINT8_MAX
#define UINT_LEAST16_MAX  UINT16_MAX
#define UINT_LEAST32_MAX  UINT32_MAX
#define UINT_LEAST64_MAX  UINT64_MAX
#endif

#ifndef	INT_FAST8_MIN
#define INT_FAST8_MIN     INT8_MIN
#define INT_FAST16_MIN    INT16_MIN
#define INT_FAST32_MIN    INT32_MIN
#define INT_FAST64_MIN    INT64_MIN
#endif

#ifndef	INT_FAST8_MAX
#define INT_FAST8_MAX     INT8_MAX
#define INT_FAST16_MAX    INT16_MAX
#define INT_FAST32_MAX    INT32_MAX
#define INT_FAST64_MAX    INT64_MAX
#endif

#ifndef	UINT_FAST8_MAX
#define UINT_FAST8_MAX    UINT8_MAX
#define UINT_FAST16_MAX   UINT16_MAX
#define UINT_FAST32_MAX   UINT32_MAX
#define UINT_FAST64_MAX   UINT64_MAX
#endif


/* defines: minimums and maximums */

#ifndef	PTRDIFF_MIN
#define PTRDIFF_MIN	INT32_MIN
#endif

#ifndef	PTRDIFF_MAX
#define PTRDIFF_MAX	INT32_MAX
#endif

#ifndef	SIG_ATOMIC_MIN
#define	SIG_ATOMIC_MIN	INT_MIN
#endif

#ifndef	SIG_ATOMIC_MAX
#define	SIG_ATOMIC_MAX	INT_MAX
#endif

#ifndef	SIZE_MAX
#define SIZE_MAX	UINT32_MAX
#endif

#ifndef	WCHAR_MIN
#define WCHAR_MIN	INT32_MIN
#endif

#ifndef	WCHAR_MAX
#define WCHAR_MAX	INT32_MAX
#endif

#ifndef	WINT_MIN
#define WINT_MIN	INT32_MIN
#endif

#ifndef	WINT_MAX
#define WINT_MAX	INT32_MAX
#endif


#endif /* if C++, then __STDC_LIMIT_MACROS enables the above macros */



/* "C++ implementations should define these macros only when
 *  __STDC_CONSTANT_MACROS is defined before <stdint.h> is included."
 */ 
#if (! defined(__cplusplus)) || defined(__STDC_CONSTANT_MACROS)

#ifndef	INT8_C
#define INT8_C(v)    ((int8_t)v)
#define INT16_C(v)   ((int16_t)v)
#define INT32_C(v)   (v ## L)
#define INT64_C(v)   (v ## LL)
#endif

#ifndef	UINT8_C
#define UINT8_C(v)   ((uint8_t)v)
#define UINT16_C(v)  ((uint16_t)v)
#define UINT32_C(v)  (v ## UL)
#define UINT64_C(v)  (v ## ULL)
#endif

#ifndef	INTMAX_C
#define INTMAX_C(v)  (v ## LL)
#define UINTMAX_C(v) (v ## ULL)
#endif

#endif /* if C++, then __STDC_CONSTANT_MACROS enables the above macros */


#endif /* _STDINT_H_ */



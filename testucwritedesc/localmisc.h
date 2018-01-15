/* localmisc */

/* miscellaneous stuff which essentially every program wants! */
/* last modified %G% version %I% */


/* revision history:

	= 1998-02-15, David A­D­ Morano
	This code was started to make life easier on the outside (outside of
	Lucent Technologies).  This file largely contains those things
	(defines) that I have found to be either useful or problematic in the
	past.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	LOCALMISC_INCLUDE
#define	LOCALMISC_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<limits.h>


#ifndef	va_begin
#define	va_begin(ap,arg)	va_start((ap),(arg))
#endif
#ifndef	va_finish
#define	va_finish(ap)		va_end((ap))
#endif
#ifndef	va_get
#define	va_get(ap,atype)	va_arg((ap),(atype))
#endif

#ifndef	repeat
#define repeat			do
#define	until(cond)		while(!(cond))
#endif

#ifndef	TRUE
#define	TRUE		1
#endif

#ifndef	FALSE
#define	FALSE		0
#endif

#ifndef	YES
#define	YES		1
#endif

#ifndef	NO
#define	NO		0
#endif

#ifndef	OK
#define	OK		0
#endif

#ifndef	BAD
#define	BAD		-1
#endif


#ifndef	MIN
#define	MIN(a,b)	(((a) < (b)) ? (a) : (b))
#endif

#ifndef	MAX
#define	MAX(a,b)	(((a) > (b)) ? (a) : (b))
#endif

#ifndef	UMIN
#define	UMIN(a,b)	((((unsigned long) (a)) < ((unsigned long) (b))) \
				 ? (a) : (b))
#endif

#ifndef	UMAX
#define	UMAX(a,b)	((((unsigned long) (a)) > ((unsigned long) (b))) \
				? (a) : (b))
#endif

#ifndef	ABS
#define	ABS(a)		(((a) < 0) ? (- (a)) : (a))
#endif

#ifndef	LEQUIV /* should be operator »  !^^  « or perhaps »  !!  « */
#define	LEQUIV(a,b)	(((a) && (b)) || ((! (a)) && (! (b))))
#endif

#ifndef	LXOR /* should be operator »  ^^  « */
#define	LXOR(a,b)	(((a) && (! (b))) || ((! (a)) && (b)))
#endif

#ifndef	BCEIL
#define	BCEIL(v,m)	(((v) + ((m) - 1)) & (~ ((m) - 1)))
#endif

#ifndef	BFLOOR
#define	BFLOOR(v,m)	((v) & (~ ((m) - 1)))
#endif

#ifndef	CEILINT
#define	CEILINT(v)	BCEIL((v),sizeof(int))
#endif

#ifndef	CMSG_SPACE
#define	CMSG_SPACE(len)	(CEILINT(sizeof(struct cmsghdr)) + CEILINT(len))
#endif

#ifndef	CMSG_LEN
#define	CMSG_LEN(len)	(CEILINT(sizeof(struct cmsghdr)) + (len))
#endif

#ifndef	MKCHAR
#define	MKCHAR(c)	((c) & 0xff)
#endif

#ifndef	MKBOOL
#define	MKBOOL(c)	((c)!=0)
#endif

#ifndef	UC
#define	UC(ch)		((unsigned char) (ch))
#endif


#ifndef	PF_LOCAL
#ifdef	PF_UNIX
#define	PF_LOCAL	PF_UNIX
#else
#define	PF_LOCAL	0
#endif
#endif

#ifndef	AF_LOCAL
#ifdef	AF_UNIX
#define	AF_LOCAL	AF_UNIX
#else
#define	AF_LOCAL	0
#endif
#endif

/* basic scalar types */

#ifndef	CHAR
#define	CHAR		char
#endif

#ifndef	BYTE
#define	BYTE		char
#endif

#ifndef	SHORT
#define	SHORT		short
#endif

#ifndef	INT
#define	INT		int
#endif

#ifndef	LONGLONG
#undef	S1
#define	S1	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
#undef	S2
#define	S2	defined(OSNAME_IRIX) && (OSNAME_IRIX > 0)
#undef	S3
#define	S3	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
#define	F	(S1 || S2 || S3)
#if	F
#define	LONGLONG	long long
#else
#define	LONGLONG	long
#endif /* (whether implementation has 'long long' or not) */
#undef	S1
#undef	S2
#undef	S3
#undef	F
#endif

#ifndef	LONG
#undef	S1
#define	S1	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
#undef	S2
#define	S2	defined(OSNAME_IRIX) && (OSNAME_IRIX > 0)
#undef	S3
#define	S3	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
#define	F1	defined(LONGLONG)
#define	F	(F1 || S1 || S2 || S3)
#if	F
#define	LONG	long long
#else
#define	LONG	long
#endif /* (whether implementation has 'long long' or not) */
#undef	S1
#undef	S2
#undef	S3
#undef	F
#endif

#ifndef	SCHAR
#define	SCHAR		signed char
#endif

#ifndef	UCHAR
#define	UCHAR		unsigned char
#endif

#ifndef	USHORT
#define	USHORT		unsigned short
#endif

#ifndef	UINT
#define	UINT		unsigned int
#endif

#ifndef	ULONG
#define	ULONG		unsigned LONG
#endif

#ifndef	ULONGLONG
#define	ULONGLONG	unsigned long long
#endif


#if	defined(OSNAME_Darwin) && defined(OSNUM) && (OSNUM <= 7)
#if	(! defined(_POSIX_SOURCE))
#ifndef	TYPEDEF_USHORT
#define	TYPEDEF_USHORT	1
#endif
#ifndef	TYPEDEF_UINT
#define	TYPEDEF_UINT	1
#endif
#endif /* (! defined(_POSIX_SOURCE)) */
#endif /* defined(OSNAME_Darwin) */


#if	(! defined(__EXTENSIONS__)) && (! defined(P_MYID))
#if	defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE)
#if	(! defined(IRIX)) || (! defined(_SYS_BSD_TYPEDEFS_H))

#ifndef	TYPEDEFS_UNSIGNED
#define	TYPEDEFS_UNSIGNED	1

#ifndef	TYPEDEF_USHORT
#define	TYPEDEF_USHORT		1
typedef unsigned short		ushort ;
#endif

#ifndef	TYPEDEF_UINT
#define	TYPEDEF_UINT		1
typedef unsigned int		uint ;
#endif

#ifndef	TYPEDEF_ULONG
#define	TYPEDEF_ULONG		1
typedef unsigned long		ulong ;
#endif

#ifndef	TYPEDEF_ULONGLONG
#define	TYPEDEF_ULONGLONG	1
typedef unsigned long long	ulonglong ;
#endif

#endif /* TYPEDEFS_UNSIGNED */

#endif
#endif
#endif


/* do it again! */

#if	(! defined(TYPEDEFS_UNSIGNED)) && (! defined(P_MYID))
#if	(! defined(IRIX)) || (! defined(_SYS_BSD_TYPEDEFS_H))

#ifndef	TYPEDEFS_UNSIGNED
#define	TYPEDEFS_UNSIGNED	1

#ifndef	TYPEDEF_USHORT
#define	TYPEDEF_USHORT		1
typedef unsigned short		ushort ;
#endif

#ifndef	TYPEDEF_UINT
#define	TYPEDEF_UINT		1
typedef unsigned int		uint ;
#endif

#ifndef	TYPEDEF_ULONG
#define	TYPEDEF_ULONG		1
typedef unsigned long		ulong ;
#endif

#ifndef	TYPEDEF_ULONGLONG
#define	TYPEDEF_ULONGLONG	1
typedef unsigned long long	ulonglong ;
#endif

#endif /* TYPEDEFS_UNSIGNED */

#endif
#endif


#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef const char		cchar ;
#endif

#ifndef	TYPEDEF_SCHAR
#define	TYPEDEF_SCHAR	1
typedef signed char		schar ;
#endif /* TYPEDEF_SCHAR */


#ifndef	TYPEDEF_CSCHAR
#define	TYPEDEF_CSCHAR	1
typedef const signed char	cschar ;
#endif /* TYPEDEF_SCHAR */


#ifndef	TYPEDEF_UCHAR
#define	TYPEDEF_UCHAR	1
typedef unsigned char		uchar ;
#endif /* TYPEDEF_UCHAR */


#ifndef	TYPEDEF_CUCHAR
#define	TYPEDEF_CUCHAR	1
typedef const unsigned char	cuchar ;
#endif /* TYPEDEF_UCHAR */


#ifndef	TYPEDEF_LONGLONG
#define	TYPEDEF_LONGLONG	1
typedef long long		longlong ;
#endif /* TYPEDEF_LONGLONG */


#ifndef	TYPEDEF_ULONGLONG
#define	TYPEDEF_ULONGLONG	1
typedef unsigned long long	ulonglong ;
#endif /* TYPEDEF_ULONGLONG */


#ifndef	TYPEDEF_LONG64
#define	TYPEDEF_LONG64	1
typedef LONG			long64 ;
#endif /* TYPEDEF_LONG64 */


#ifndef	TYPEDEF_ULONG64
#define	TYPEDEF_ULONG64	1
typedef ULONG			ulong64 ;
#endif /* TYPEDEF_ULONG64 */


#if	defined(SYSHAS_OFFSET) && (SYSHAS_OFFSET > 0)
#define	TYPEDEF_OFFSET		1
#else
#ifndef	TYPEDEF_OFFSET
#define	TYPEDEF_OFFSET		1
typedef long long		offset_t ;
#endif
#endif


#if	defined(SYSHAS_OFF32) && (SYSHAS_OFF32 > 0)
#define	TYPEDEF_OFF32		1
#else
#ifndef	TYPEDEF_OFF32
#define	TYPEDEF_OFF32		1
typedef int			off32_t ;
#endif /* TYPEDEF_OFF32 */
#endif


#ifndef	TYPEDEF_USTIME
#define	TYPEDEF_USTIME		1
typedef unsigned int		ustime_t ;
#endif

#ifndef	TYPEDEF_UTIME
#define	TYPEDEF_UTIME		1
#if	defined(_LP64)
typedef unsigned long		utime_t ;
#else
typedef unsigned long long	utime_t ;
#endif
#endif /* TYPEDEF_UTIME */

#ifndef	TYPEDEF_UNIXTIME
#define	TYPEDEF_UNIXTIME	1
#if	defined(_LP64)
typedef long			unixtime_t ;
#else
typedef long long		unixtime_t ;
#endif
#endif /* TYPEDEF_UNIXTIME */


/* C-language limits */

#ifndef	LONGLONG_MIN
#define	LONGLONG_MIN	(-9223372036854775807LL-1LL)
#endif /* LONGLONG_MIN */
#ifndef	LONGLONG_MAX
#define	LONGLONG_MAX	9223372036854775807LL
#endif /* LONGLONG_MAX */
#ifndef	ULONGLONG_MAX
#define	ULONGLONG_MAX	18446744073709551615ULL
#endif /* ULONGLONG_MAX */

/* some limits! (C-language) */

#ifndef	LONG64_MIN
#define	LONG64_MIN	(-9223372036854775807L-1LL)
#endif

#ifndef	LONG64_MAX
#define	LONG64_MAX	9223372036854775807LL
#endif

#ifndef	ULONG64_MAX
#define	ULONG64_MAX	18446744073709551615ULL
#endif

/* it would be nice if the implemenation had these */

#ifndef	SHORT_MIN
#ifdef	SHRT_MIN
#define	SHORT_MIN	SHRT_MIN
#else
#define	SHORT_MIN	(-32768)	/* min value of a "short int" */
#endif
#endif

#ifndef	SHORT_MAX
#ifdef	SHRT_MAX
#define	SHORT_MAX	SHRT_MAX
#else
#define	SHORT_MAX	32767		/* max value of a "short int" */
#endif
#endif

#ifndef	USHORT_MAX
#ifdef	USHRT_MAX
#define	USHORT_MAX	USHRT_MAX
#else
#define	USHORT_MAX	65535		/* max value of "unsigned short int" */
#endif
#endif


/* parameters */

#ifndef	MSGBUFLEN
#define	MSGBUFLEN	2048
#endif

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

/* timezone (zoneinfo) name */
#ifndef	TZLEN
#define	TZLEN		60
#endif

/* timezone abbreviation */
#ifndef	ZNAMELEN
#define	ZNAMELEN	8
#endif
#ifndef	TZABBRLEN
#define	TZABBRLEN	8
#endif

/* log-ID (for logging) */
#ifndef	LOGIDLEN
#define	LOGIDLEN	15
#endif

/* mail address */
#ifndef	MAILADDRLEN
#define	MAILADDRLEN	(3 * MAXHOSTNAMELEN)
#endif

#ifndef	NOFILE
#define	NOFILE		20
#endif

#ifndef	INET4_ADDRSTRLEN
#ifdef	INET_ADDRSTRLEN
#define	INET4_ADDRSTRLEN	INET_ADDRSTRLEN
#else
#define	INET4_ADDRSTRLEN	16
#endif
#endif

#ifndef	INET6_ADDRSTRLEN
#define	INET6_ADDRSTRLEN	46
#endif

#ifndef	INETX_ADDRSTRLEN
#define	INETX_ADDRSTRLEN	MAX(INET4_ADDRSTRLEN,INET6_ADDRSTRLEN)
#endif


/* language features */

#ifndef	LANGUAGE_FOREVER
#define	LANGUAGE_FOREVER	1
#define	forever		for (;;)
#endif /* LANGUAGE_FOREVER */

#ifndef	LANGUAGE_NELEMENTS
#define	LANGUAGE_NELEMENTS	1
#ifndef	nelements
#define	nelements(n)	(sizeof(n) / sizeof((n)[0]))
#endif
#endif /* LANGUAGE_NELEMENTS */

#ifndef	LANGUAGE_NELEM
#define	LANGUAGE_NELEM		1
#ifndef	nelem
#ifdef	nelements
#define	nelem		nelements
#else
#define	nelem(n)	(sizeof(n) / sizeof((n)[0]))
#endif
#endif
#endif /* LANGUAGE_NELEM */

#ifndef	NULL
#define	NULL		0
#endif

#ifndef	VOID
#define	VOID		void
#endif

#ifndef	VOLATILE
#define	VOLATILE	volatile
#endif

#ifndef	CONST
#define	CONST		const
#endif

#ifndef	RESTRICT
#define	RESTRICT	/* restrict */
#endif

#ifndef	STRUCT_MAPEX
#define	STRUCT_MAPEX	1

#ifndef	MAPEX
#define	MAPEX		struct mapex
#endif

struct mapex {
	int	rs, ex ;
} ;

#endif /* STRUCT_MAPEX */


#ifndef	EXTERN_MAXEX
#define	EXTERN_MAXEX	1

#ifdef	__cplusplus
extern "C" {
#endif

extern int	mapex(const struct mapex *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* EXTERN_MAXPEX */


#if	defined(SYSHAS_STRNLEN) && (SYSHAS_STRNLEN > 0)
#else
#ifdef	__cplusplus
extern "C" {
#endif

extern int	strnlen(const char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* defined(SYSHAS_STRNLEN) */


/* names */

#ifndef	STDNULLFNAME
#define	STDNULLFNAME	"*STDNULL*"
#endif

#ifndef	STDINFNAME
#define	STDINFNAME	"*STDIN*"
#endif

#ifndef	STDOUTFNAME
#define	STDOUTFNAME	"*STDOUT*"
#endif

#ifndef	STDERRFNAME
#define	STDERRFNAME	"*STDERR*"
#endif

#ifndef	NULLFNAME
#define	NULLFNAME	"/dev/null"
#endif

#ifndef	POLLINTMULT
#define	POLLINTMULT	1000		/* poll-time multiplier */
#endif

#ifndef	REALNAMELEN
#define	REALNAMELEN	100
#endif

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif


#endif /* LOCALMISC_INCLUDE */



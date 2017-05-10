
#if	(! defined(__EXTENSIONS__)) && (! defined(P_MYID))
#if	defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE)

#ifndef	TYPES_UNSIGNED
#define	TYPES_UNSIGNED	1

typedef unsigned short	ushort ;
typedef unsigned int	uint ;
typedef unsigned long	ulong ;

#define	TYPE_USHORT	1
#define	TYPE_UINT	1
#define	TYPE_ULONG	1

#endif /* TYPES_UNSIGNED */

#endif
#endif



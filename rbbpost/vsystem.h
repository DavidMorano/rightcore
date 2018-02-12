/* vsystem */

/* last modified %G% version %I% */
/* virtual-system definitions */


/* revision history:

	= 1998-03-21, David A­D­ Morano
	This module was originally written.

	= 2017-08-01, David A­D­ Morano
	Updated for lack of interfaces in MacOS Darwin

*/

/* Copyright © 1998,2017 David A­D­ Morano.  All rights reserved. */

#ifndef	VSYSTEM_INCLUDE
#define	VSYSTEM_INCLUDE		1


#include	"envstandards.h"	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/utsname.h>
#include	<sys/uio.h>
#include	<sys/time.h>		/* for 'u_adjtime(3u)' */
#include	<sys/timeb.h>		/* for 'uc_ftime(3uc)' */

#if	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
#include	<sys/resource.h>
#else
#include	<sys/resource.h>
#endif

#include	<sys/stat.h>
#include	<sys/statvfs.h>
#include	<sys/socket.h>
#include	<sys/poll.h>

#if	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
#include	<sys/dirent.h>
#include	<sys/dir.h>
#else
#include	<sys/dirent.h>
#endif

#if	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
#include	<sys/signal.h>
#include	<ucontext.h>
#else
#include	<ucontext.h>
#endif

#if	defined(SYSHAS_ACL) && (SYSHAS_ACL > 0)
#include	<sys/acl.h>
#endif

#include	<signal.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<utime.h>		/* for 'u_utime(2)' */
#include	<pthread.h>
#include	<termios.h>
#include	<time.h>
#include	<errno.h>
#include	<netdb.h>
#include	<pwd.h>
#include	<grp.h>

#if	defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0)
#include	<stropts.h>
#endif

#if	defined(SYSHAS_XTI) && (SYSHAS_XTI > 0)
#include	<xti.h>
#endif

#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
#include	<thread.h>
#include	<shadow.h>
#include	<project.h>
#include	<user_attr.h>
#endif


/* UNIX® system defines */

#ifndef	SIZE_MAX
#define	SIZE_MAX	ULONG_MAX
#endif

#ifndef	SSIZE_MAX
#define	SSIZE_MAX	LONG_MAX
#endif

#ifndef	CLUSTERNAMELEN
#if	defined(NODENAMELEN)
#define	CLUSTERNAMELEN	NODENAMELEN
#else
#define	CLUSTERNAMELEN	512
#endif
#endif

#ifndef	NODENAMELEN
#if	defined(SYS_NMLN)
#define	NODENAMELEN	(SYS_NMLN-1)	/* usually 256 for SVR4! */
#else
#define	NODENAMELEN	256		/* should be at least 256 for SVR4! */
#endif
#endif

#ifndef	USERNAMELEN
#ifdef	LOGNAME_MAX
#define	USERNAMELEN	LOGNAME_MAX
#else
#define	USERNAMELEN	32
#endif
#endif

#ifndef	GROUPNAMELEN
#ifndef	LOGNAME_MAX
#define	GROUPNAMELEN	LOGNAME_MAX
#else
#define	GROUPNAMELEN	32
#endif
#endif

/* Solaris® project name */
#ifndef	PROJNAMELEN
#ifndef	LOGNAME_MAX
#define	PROJNAMELEN	LOGNAME_MAX
#else
#define	PROJNAMELEN	32
#endif
#endif

#ifndef	LOGNAMELEN
#ifdef	LOGNAME_MAX
#define	LOGNAMELEN	LOGNAME_MAX
#else
#define	LOGNAMELEN	32
#endif
#endif

#ifndef	PASSWORDLEN
#define	PASSWORDLEN	8
#endif

/* service name */
#ifndef	SVCNAMELEN
#define	SVCNAMELEN	32
#endif

#ifndef	SIGACTION
#define	SIGACTION	struct sigaction
#endif

#ifndef	SIGEVENT
#define	SIGEVENT	struct sigevent
#endif

#ifndef	RLIMIT
#define	RLIMIT		struct rlimit
#endif

#ifndef	USTAT
#define	USTAT		struct ustat
#endif

#ifndef	USTATVFS
#define	USTATVFS	struct ustatvfs
#endif

#ifndef	SOCKADDR
#define	SOCKADDR	struct sockaddr
#endif

#ifndef	CMSGHDR
#define	CMSGHDR		struct cmsghdr
#endif

#ifndef	MSGHDR
#define	MSGHDR		struct msghdr
#endif

#ifndef	WINSIZE
#define	WINSIZE		struct winsize
#endif

#ifndef	TIMEVAL
#define	TIMEVAL		struct timeval
#endif

#ifndef	TIMESPEC
#define	TIMESPEC	struct timespec
#endif

#ifndef	ITIMERSPEC
#define	ITIMERSPEC	struct itimerspec
#endif

#ifndef	UTIMBUF
#define	UTIMBUF		struct utimbuf
#endif

#ifndef	TIMEB
#define	TIMEB		struct timeb
#endif

#ifndef	ADDRINFO
#define	ADDRINFO	struct addrinfo
#endif

#ifndef	HOSTENT
#define	HOSTENT		struct hostent
#endif

#ifndef	TERMIOS
#define	TERMIOS		struct termios
#endif

#ifndef	STRBUF
#define	STRBUF		struct strbuf
#endif

#ifndef	POLLFD
#define	POLLFD		struct pollfd
#endif

#ifndef	STRRECVFD
#define	STRRECVFD	struct strrecvfd
#endif

#ifndef	IOVCEC
#define	IOVCEC		struct iovec
#endif


/* for missing MAXNAMELEN */

#undef	S1
#define	S1	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
#if	S1 && (! defined(MAXNAMELEN))
#define	MAXNAMELEN	256
#endif


/* possible missing S_ISNAM */

#ifndef	S_ISNAM
#ifdef	S_IFNAM
#define	S_ISNAM(mode)	(((mode)&S_IFMT) == S_IFNAM)
#else
#define	S_ISNAM(mode)	0
#endif
#endif /* S_ISNAM */


/* possible missing protocol-families and address-families */

#ifndef	PF_INET4
#ifdef	PF_INET
#define	PF_INET4	PF_INET
#else
#define	PF_INET4	2
#endif
#endif

#ifndef	AF_INET4
#ifndef	AF_INET
#define	AF_INET4	2
#else
#define	AF_INET4	AF_INET
#endif
#endif

#ifndef	SIGTIMEOUT
#define	SIGTIMEOUT	SIGRTMAX
#endif


/* for 'stat(2)' and its many friends */

#ifndef	STRUCT_USTAT
#define	STRUCT_USTAT	1
#if	defined(_LARGEFILE_SOURCE)
#define	ustat	stat
#else
#if	defined(_LARGEFILE64_SOURCE)
#define	ustat	stat64
#else
#define	ustat	stat
#endif
#endif
#endif


/* for 'statvfs(2)' and its many friends */

#ifndef	STRUCT_USTATVFS
#define	STRUCT_USTATVFS	1
#if	defined(_LARGEFILE_SOURCE)
#define	ustatvfs	statvfs
#else
#if	defined(_LARGEFILE64_SOURCE)
#define	ustatvfs	statvfs64
#else
#define	ustatvfs	statvfs
#endif
#endif
#endif


/* general INODE type */

#ifndef	TYPEDEF_INODE
#define	TYPEDEF_INODE	1
#if	defined(_FILE_OFFSET_BITS) && (_FILE_OFFSET_BITS == 64)
typedef ino_t	uino_t ;
#else
#if	defined(_LARGEFILE64_SOURCE)
typedef ino64_t	uino_t ;
#else
typedef ino_t	uino_t ;
#endif
#endif
#endif


/* offset_t */

#if	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
#ifndef	_OFFSET_T
#define	_OFFSET_T	1
#ifndef	TYPEDEF_OFFSET
#define	TYPEDEF_OFFSET	1
#if	defined(_FILE_OFFSET_BITS) && (_FILE_OFFSET_BITS == 64)
typedef long long	offset_t ;
#else
#if	defined(_LARGEFILE64_SOURCE)
typedef off_t		offset_t ;
#else
typedef off_t		offset_t ;
#endif
#endif
#endif /* TYPEDEF_OFFSET */
#endif /* _OFFSET_T */
#endif /* defined(OSNAME_Darwin) && (OSNAME_Darwin > 0) */


/* the long integer problem! */

#ifndef	LONG
#undef	S1
#define	S1	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
#undef	S2
#define	S2	defined(OSNAME_IRIX) && (OSNAME_IRIX > 0)
#undef	S3
#define	S3	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
#define	F1	(S1 || S2 || S3)

#if	(LONG_BIT == 64)
#define	LONG	long
#else /* (LONG_BIT == 64) */
#if	F1 || defined(LONGLONG)
#define	LONG	long long
#else
#define	LONG	long
#endif /* SunOS */
#endif /* (LONG_BIT == 64) */

#undef	F1
#endif


/* types */

#ifndef	TYPEDEF_LONG64
#if	defined(LONG)
#define	TYPEDEF_LONG64	1
typedef LONG	long64 ;
#endif
#endif /* TYPEDEF_LONG64 */


#ifndef	TYPEDEF_ULONG64
#if	defined(ULONG)
#define	TYPEDEF_ULONG64	1
typedef ULONG	ulong64 ;
#endif
#endif /* TYPEDEF_ULONG64 */


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


/* handle some really brain-damaged systems -- like MacOS-X Darwin®! */

#if	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
#if	defined(OSNUM) && (OSNUM <= 7)
typedef int			id_t ;
#endif
#endif

#if	defined(OSNAME_Darwin) && (OSNAME_Darwin > 0)
#if	defined(OSNUM) && (OSNUM <= 9)
typedef struct in6_addr		in6_addr_t ;
#endif
#endif


#ifndef	ULONG
#define	ULONG		unsigned LONG
#endif


/* some limits! */

#ifndef	LONG64_MIN
#define	LONG64_MIN	(-9223372036854775807LL-1LL)
#endif

#ifndef	LONG64_MAX
#define	LONG64_MAX	9223372036854775807LL
#endif

#ifndef	ULONG64_MAX
#define	ULONG64_MAX	18446744073709551615ULL
#endif

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

#ifndef	TIME_MAX
#define TIME_MAX	INT_MAX
#endif

#ifndef	TIME_MIN
#define	TIME_MIN	INT_MIN
#endif


/* symbol synonyms */

#ifndef	va_begin
#define	va_begin(ap,arg)	va_start((ap),(arg))
#endif
#ifndef	va_finish
#define	va_finish(ap)		va_end((ap))
#endif
#ifndef	va_get
#define	va_get(ap,atype)	va_arg((ap),(atype))
#endif


/* extra system flags for 'openXXX(2)' and friends */

#ifndef	OM_SPECIAL
#define	OM_SPECIAL	0xFF000000
#endif

#ifndef	O_SETSID
#define	O_SETSID	(1<<24)
#endif
#ifndef	O_CLOEXEC
#define	O_CLOEXEC	(1<<25)
#endif
#ifndef	O_NETWORK
#define	O_NETWORK	(1<<26)
#endif
#ifndef	O_MINMODE
#define	O_MINMODE	(1<<27)
#endif


/* extra system flags for |uc_lockfile(3uc)| */

#ifndef	F_UNLOCK
#define	F_UNLOCK	F_ULOCK
#endif
#ifndef	F_WLOCK	
#define	F_WLOCK		F_LOCK
#endif
#ifndef	F_TWLOCK
#define	F_TWLOCK	F_TLOCK
#endif
#ifndef	F_WTEST
#define	F_WTEST		F_TEST
#endif
#ifndef	F_RLOCK
#define	F_RLOCK		10		/* new! (watch UNIX® for changes) */
#endif
#ifndef	F_TRLOCK
#define	F_TRLOCK	11		/* new! (watch UNIX® for changes) */
#endif
#ifndef	F_RTEST
#define	F_RTEST		12		/* new! (watch UNIX® for changes) */
#endif


/* UNIX® System return-status codes */

/* status return codes (only used when explicit return status is requested) */

#define	SR_TIMEOUT	2		/* timed out w/ possible data */
#define	SR_CREATED	1		/* created object */

/* operation returns */

#define	SR_OK		0		/* the OK return */

#define	SR_PERM		(- EPERM)	/* Not super-user	*/ 
#define	SR_NOENT	(- ENOENT)	/* No such file or directory */
#define	SR_SRCH		(- ESRCH)	/* No such process */
#define	SR_INTR		(- EINTR)	/* interrupted system call */
#define	SR_IO		(- EIO)		/* I/O error */
#define	SR_NXIO		(- ENXIO)	/* No such device or address */
#define	SR_2BIG		(- E2BIG)	/* Arg list too long */
#define	SR_NOEXEC	(- ENOEXEC)	/* Exec format error */
#define	SR_BADF		(- EBADF)	/* Bad file number */
#define	SR_CHILD	(- ECHILD)	/* No children */
#define	SR_AGAIN	(- EAGAIN)	/* Resource temporarily unavailable */
#define	SR_NOMEM	(- ENOMEM)	/* Not enough core */
#define	SR_ACCES	(- EACCES)	/* Permission denied */
#define	SR_FAULT	(- EFAULT)	/* Bad address */
#define	SR_NOTBLK	(- ENOTBLK)	/* Block device required */
#define	SR_BUSY		(- EBUSY)	/* Mount device busy */
#define	SR_EXIST	(- EEXIST)	/* File exists */
#define	SR_XDEV		(- EXDEV)	/* Cross-device link */
#define	SR_NODEV	(- ENODEV)	/* No such device */
#define	SR_NOTDIR	(- ENOTDIR)	/* Not a directory */
#define	SR_ISDIR	(- EISDIR)	/* Is a directory */
#define	SR_INVAL	(- EINVAL)	/* Invalid argument */
#define	SR_NFILE	(- ENFILE)	/* File table overflow */
#define	SR_MFILE	(- EMFILE)	/* Too many open files */
#define	SR_NOTTY	(- ENOTTY)	/* Inappropriate ioctl for device */
#define	SR_TXTBSY	(- ETXTBSY)	/* Text file busy */
#define	SR_FBIG		(- EFBIG)	/* File too large */
#define	SR_NOSPC	(- ENOSPC)	/* No space left on device */
#define	SR_SPIPE	(- ESPIPE)	/* Illegal seek */
#define	SR_ROFS		(- EROFS)	/* Read only file system */
#define	SR_MLINK	(- EMLINK)	/* Too many links */
#define	SR_PIPE		(- EPIPE)	/* Broken pipe */
#define	SR_DOM		(- EDOM)	/* Math arg out of domain of func */
#define	SR_RANGE	(- ERANGE)	/* Math result not representable */
#define	SR_NOMSG	(- ENOMSG)	/* No message of desired type */
#define	SR_IDRM		(- EIDRM)	/* Identifier removed */
#define	SR_CHRNG	(- ECHRNG)	/* Channel number out of range */
#define	SR_L2NSYNC	(- EL2NSYNC)	/* Level 2 not synchronized */
#define	SR_L3HLT	(- EL3HLT)	/* Level 3 halted */
#define	SR_L3RST	(- EL3RST)	/* Level 3 reset */
#define	SR_LNRNG	(- ELNRNG)	/* Link number out of range */
#define	SR_UNATCH	(- EUNATCH)	/* Protocol driver not attached */
#define	SR_NOCSI	(- ENOCSI)	/* No CSI structure available */
#define	SR_L2HLT	(- EL2HLT)	/* Level 2 halted */
#define	SR_DEADLK	(- EDEADLK)	/* Deadlock condition */
#define	SR_NOLCK	(- ENOLCK)	/* No record locks available */
#define	SR_CANCELED	(- ECANCELED)	/* Operation canceled */
#define	SR_NOTSUP	(- ENOTSUP)	/* Operation not supported */
#define	SR_DQUOT	(- EDQUOT)	/* Disc quota exceeded */
#define	SR_BADE		(- EBADE)	/* invalid exchange */
#ifdef	EBADR
#define	SR_BADR		(- EBADR)	/* invalid request descriptor */
#else
#define	SR_BADR		-1000
#endif
#define	SR_XFULL	(- EXFULL)	/* exchange full */
#ifdef	ENOANO
#define	SR_NOANO	(- ENOANO)	/* no anode */
#else
#define	SR_NOANO	-1001
#endif /* ENOANO */
#ifdef	EBADRQC
#define	SR_BADRQC	(- EBADRQC)	/* invalid request code */
#else
#define	SR_BADRQC	-1002
#endif
#ifdef	EBADSLT
#define	SR_BADSLT	(- EBADSLT)	/* invalid slot */
#else
#define	SR_BADSLT	-1003
#endif /* EBADSLT */
#ifdef	EDEADLOCK
#define	SR_DEADLOCK	(- EDEADLOCK)	/* file locking deadlock error */
#else
#define	SR_DEADLOCK	-1004
#endif

#ifdef	EBFONT
#define	SR_BFONT	(- EBFONT)	/* bad font file fmt */
#else
#define	SR_BFONT	-1004
#endif /* EBFONT */

#ifdef	EOWNERDEAD
#define	SR_OWNERDEAD	(- EOWNERDEAD)	/* owner dead (for mutexes) */
#else
#define	SR_OWNERDEAD	-1005
#endif /* EBFONT */

#ifdef	ENOTRECOVERABLE
#define	SR_NOTRECOVERABLE	(- ENOTRECOVERABLE)	/* not recoverable */
#else
#define	SR_NOTRECOVERABLE	-1006
#endif /* ENOTRECOVERABLE */

#define	SR_NOSTR	(- ENOSTR)	/* Device not a stream */
#define	SR_NODATA	(- ENODATA)	/* no data (for no-delay io) */
#define	SR_TIME		(- ETIME)	/* timer expired */
#ifdef	ENOSR
#define	SR_NOSR		(- ENOSR)	/* out of streams resources */
#else
#define	SR_NOSR		-1007
#endif
#define	SR_NONET	(- ENONET)	/* Machine is not on the network */
#ifdef	ENOPKG
#define	SR_NOPKG	(- ENOPKG)	/* Package not installed */
#else
#define	SR_NOPKG	-1008
#endif
#define	SR_REMOTE	(- EREMOTE)	/* The object is remote */
#define	SR_NOLINK	(- ENOLINK)	/* the link has been severed */
#define	SR_ADV		(- EADV)	/* advertise error */
#define	SR_SRMNT	(- ESRMNT)	/* srmount error */
#define	SR_COMM		(- ECOMM)	/* Communication error on send */
#ifdef	EPROTO
#define	SR_PROTO	(- EPROTO)	/* Protocol error */
#else
#define	SR_PROTO	-1009
#endif
#define	SR_MULTIHOP	(- EMULTIHOP)	/* multihop attempted */
#define	SR_BADMSG	(- EBADMSG)	/* trying to read unreadable message */
#define	SR_NAMETOOLONG	(- ENAMETOOLONG)	/* path name is too long */
#define	SR_OVERFLOW	(- EOVERFLOW)	/* value too large to be stored */
#define	SR_NOTUNIQ	(- ENOTUNIQ)	/* given login name not unique */
#ifdef	EBADFD
#define	SR_BADFD	(- EBADFD)	/* FD invalid for this operation */
#else
#define	SR_BADFD	-1010
#endif
#define	SR_REMCHG	(- EREMCHG)	/* Remote address changed */
#ifdef	ELIBACC
#define	SR_LIBACC	(- ELIBACC)	/* Can't access a needed shared lib */
#else
#define	SR_LIBACC	-1011
#endif
#define	SR_LIBBAD	(- ELIBBAD)	/* Accessing a corrupted shared lib */
#define	SR_LIBSCN	(- ELIBSCN)	/* .lib section in a.out corrupted */
#define	SR_LIBMAX	(- ELIBMAX)	/* Attempting link in too many libs */
#define	SR_LIBEXEC	(- ELIBEXEC)	/* Attempting to exec shared library */
#define	SR_ILSEQ	(- EILSEQ)	/* Illegal byte sequence */
#define	SR_NOSYS	(- ENOSYS)	/* Unsupported file system operation */
#define	SR_LOOP		(- ELOOP)	/* Symbolic link loop */
#define	SR_RESTART	(- ERESTART)	/* Restartable system call */
#define	SR_STRPIPE	(- ESTRPIPE)	/* if pipe/FIFO, don't sleep */
#define	SR_NOTEMPTY	(- ENOTEMPTY)	/* directory not empty */
#define	SR_USERS	(- EUSERS)	/* Too many users (for UFS) */
#define	SR_NOTSOCK	(- ENOTSOCK)	/* Socket operation on non-socket */
#define	SR_DESTADDRREQ	(- EDESTADDRREQ)	/* dst address required */
#define	SR_MSGSIZE	(- EMSGSIZE)	/* Message too long */
#define	SR_PROTOTYPE	(- EPROTOTYPE)	/* Protocol wrong type for socket */
#define	SR_NOPROTOOPT	(- ENOPROTOOPT)	/* Protocol not available */
#define	SR_PROTONOSUPPORT	(- EPROTONOSUPPORT)	/* proto not sup. */
#define	SR_SOCKTNOSUPPORT	(- ESOCKTNOSUPPORT) /* Socket not supported */
#define	SR_OPNOTSUPP	(- EOPNOTSUPP)	/* Operation not supported on socket */
#define	SR_PFNOSUPPORT	(- EPFNOSUPPORT)	/* proto family not supported */
#define	SR_AFNOSUPPORT	(- EAFNOSUPPORT)	/* AF not supported by */
#define	SR_ADDRINUSE	(- EADDRINUSE)	/* Address already in use */
#define	SR_ADDRNOTAVAIL	(- EADDRNOTAVAIL)	/* Can't assign address */
#define	SR_NETDOWN	(- ENETDOWN)	/* Network is down */
#define	SR_NETUNREACH	(- ENETUNREACH)	/* Network is unreachable */
#define	SR_NETRESET	(- ENETRESET)	/* Network dropped connection reset */
#define	SR_CONNABORTED	(- ECONNABORTED)	/* connection abort */
#define	SR_CONNRESET	(- ECONNRESET)	/* Connection reset by peer */
#define	SR_NOBUFS	(- ENOBUFS)	/* No buffer space available */
#define	SR_ISCONN	(- EISCONN)	/* Socket is already connected */
#define	SR_NOTCONN	(- ENOTCONN)	/* Socket is not connected */
#define	SR_SHUTDOWN	(- ESHUTDOWN)	/* Can't send after socket shutdown */
#define	SR_TOOMANYREFS	(- ETOOMANYREFS)	/* Too many: can't splice */
#define	SR_TIMEDOUT	(- ETIMEDOUT)	/* Connection timed out */
#define	SR_CONNREFUSED	(- ECONNREFUSED)	/* Connection refused */
#ifdef	EHOSTDOWN
#define	SR_HOSTDOWN	(- EHOSTDOWN)	/* Host is down */
#else
#define	SR_HOSTDOWN	-1012
#endif
#define	SR_HOSTUNREACH	(- EHOSTUNREACH)	/* No route to host */
#define	SR_WOULDBLOCK	(- EAGAIN)	/* UNIX® synonym for AGAIN */
#define	SR_ALREADY	(- EALREADY)	/* operation already in progress */
#define	SR_INPROGRESS	(- EINPROGRESS)	/* operation now in progress */
#define	SR_STALE	(- ESTALE)	/* Stale NFS file handle */

#define	SR_BAD		-1013		/* general bad */

/* our favorite aliases */

#define	SR_QUIT		SR_L2HLT	/* quit requested */
#define	SR_EXIT		SR_L3HLT	/* exit requested */
#define	SR_NOENTRY	SR_NOENT	/* no entry */
#define	SR_NOTOPEN	SR_BADF		/* not open */
#define	SR_WRONLY	SR_BADF		/* not readable (historical) */
#define	SR_RDONLY	SR_BADF		/* not writable (historical) */
#define	SR_NOTSEEK	SR_SPIPE	/* not seekable */
#define	SR_ACCESS	SR_ACCES	/* permission denied */
#define	SR_INVALID	SR_INVAL	/* invalid argument */
#define	SR_EXISTS	SR_EXIST	/* object already exists */
#define	SR_LOCKED	SR_AGAIN	/* object is already locked */
#define	SR_INUSE	SR_ADDRINUSE	/* already in use */
#define	SR_LOCKLOST	SR_NXIO		/* a lock was lost */
#define	SR_HANGUP	SR_NXIO		/* hangup on device (not writable) */
#define	SR_POLLERR	SR_IO		/* SPECIAL for 'poll(2)' error */
#define	SR_TOOBIG	SR_2BIG		/* arguments too big */
#define	SR_BADFMT	SR_BFONT	/* invalid format */
#define	SR_FULL		SR_XFULL	/* object is full */
#define	SR_EMPTY	SR_NOENT	/* object is empty */
#define	SR_EOF		SR_NOENT	/* end-of-file */
#define	SR_NOEXIST	SR_NOENT	/* object does not exist */
#define	SR_NOTFOUND	SR_NOENT	/* not found */
#define	SR_BADREQUEST	SR_BADRQC	/* bad request code */
#define	SR_NOTCONNECTED	SR_NOTCONN	/* Socket is not connected */
#define	SR_OPEN		SR_ALREADY	/* already open */
#define	SR_OUT		SR_INPROGRESS	/* operation in progress */
#define	SR_NOTAVAIL	SR_ADDRNOTAVAIL	/* not available */
#define	SR_BADSLOT	SR_BADSLT	/* invalid slot */
#define	SR_SEARCH	SR_SRCH		/* No such process */
#define	SR_NOANODE	SR_NOANO	/* no anode! */
#define	SR_BUGCHECK	SR_NOANO	/* no anode! */
#define	SR_LOOK		SR_BADRQC	/* look-alert */
#define	SR_DOWN		SR_L2HLT	/* service down */
#define	SR_UNAVAIL	SR_L3HLT	/* service unavailable */
#define	SR_TXTBUSY	SR_TXTBSY	/* Text file busy */


/* open-information */

struct ucopeninfo {
	const char	**envv ;
	const char	*fname ;
	mode_t		operms ;
	int		clinks ;
	int		oflags ;
	int		to ;
	int		opts ;
} ;

/* control function codes */

#define	FC_MASK		0x0000000F	/* control function mask */

#define	fc_mask		FC_MACH

/* control functions */

#define	FC_NOOP		0
#define	FC_SETMODE	1		/* set terminal mode */
#define	FC_GETMODE	2		/* retrieve current mode */
#define	FC_REESTABLISH	3		/* terminal re-establish */
#define	FC_SETRTERMS	4		/* set read terminators */
#define	FC_GETSID	5		/* get control-terminal session ID */
#define	FC_GETPGRP	6		/* get control-terminal PGRP-ID */
#define	FC_SETPGRP	7		/* set control-term PGID */
#define	FC_GETPOP	8		/* get terminal POP information */
#define	FC_SETPOP	9		/* set terminal POP information */

#define	fc_noop		FC_NOOP
#define	fc_setmode	FC_SETMODE
#define	fc_getmode	FC_GETMODE
#define	fc_reestablish	FC_REESTABLISH
#define	fc_setrterms	FC_SETRTERMS	/* set read terminators */
#define	fc_getsid	FC_GETSID
#define	fc_getpgrp	FC_GETPGRP
#define	fc_setpgrp	FC_SETPGRP
#define	fc_getpop	FC_GETPOP
#define	fc_setpop	FC_SETPOP

/* function modifiers */

#define	FM_CMDMASK	0x0F
#define	FM_OPTMASK	0xFFFFFFF0
#define	FM_MASK		FM_OPTMASK

#define	fm_mask		FM_MASK

/* function modifiers (|open| and |setmode|) */

#define	FM_LARGEFILE	(1<<(0+4))
#define	FM_QUEUE	(1<<(1+4))
#define	FM_NOREPORT	(1<<(2+4))
#define	FM_INT		(1<<(2+4))	/* set interrupt attention */
#define	FM_KILL		(1<<(3+4))	/* set control-Y attention */
#define	FM_READATT	(1<<(5+4))	/* read-attention */
#define	FM_WRITEATT	(1<<(6+4))	/* write-attention */
#define	FM_GETSTATE	(1<<(7+4))
#define	FM_CLEARSTATE	(1<<(8+4))
#define	FM_NOSIG	(1<<(9+4))	/* do not generate signals */
#define	FM_NOSIGECHO	(1<<(10+4))	/* no echo of signals */
#define	FM_NOFLOW	(1<<(11+4))	/* no output flow-control */
#define	FM_NET		(1<<(14+4))	/* specify network operation */

#define	fm_queue	FM_QUEUE	
#define	fm_noreport	FM_NOREPORT	
#define	fm_largefile	FM_LARGEFILE
#define	fm_int		FM_INT
#define	fm_kill		FM_KILL
#define	fm_readatt	FM_READATT
#define	fm_writeatt	FM_WRITEATT
#define	fm_getstate	FM_GETSTATE
#define	fm_clearstate	FM_CLEARSTATE
#define	fm_nosig	FM_NOSIG
#define	fm_nosigecho	FM_NOSIGECHO
#define	fm_noflow	FM_NOFLOW	/* no output flow control */
#define	fm_net		FM_NET

/* function modifiers (|read|) */

#define	FM_NONE		0x0000		/* no options specified */
#define	FM_ECHO		0x0000		/* echo data */
#define	FM_FILTER	0x0000		/* do canonical processing */
#define	FM_NOECHO	(1<<(0+4))	/* no echo on input */
#define	FM_NOTECHO	(1<<(1+4))	/* no terminator echo */
#define	FM_NOFILTER	(1<<(2+4))	/* no filter on input */
#define	FM_RAWIN	(1<<(3+4))	/* raw input (no control) */
#define	FM_NONBLOCK	(1<<(4+4))	/* don't start if no data */
#define	FM_TIMED	(1<<(5+4))	/* timed read */
#define	FM_EXACT	(1<<(6+4))	/* exact transfer length */
#define	FM_AGAIN	(1<<(7+4))	/* return AGAIN if no data */
#define	FM_SLOWPOLL	(1<<(8+4))	/* perform slower polling */
#define	FM_TIMEINT	(1<<(9+4))	/* set time-out interval */
#define	FM_SETMSG	(1<<(10+4))	/* set messages on terminal (what?) */
#define	FM_NOFILTECHO	(1<<(11+4))	/* do not echo filter operations */

#define	fm_none		FM_NONE
#define	fm_echo		FM_ECHO
#define	fm_filter	FM_FILTER
#define	fm_noecho	FM_NOECHO
#define	fm_notecho	FM_NOTECHO
#define	fm_nofilter	FM_NOFILTER
#define	fm_rawin	FM_RAWIN
#define	fm_nonblock	FM_NONBLOCK
#define	fm_timed	FM_TIMED
#define	fm_exact	FM_EXACT
#define	fm_again	FM_AGAIN
#define	fm_slowpoll	FM_SLOWPOLL
#define	fm_timeint	FM_TIMEINT	/* set time-out interval */
#define	fm_setmsg	FM_SETMSG	/* what is this? */
#define	fm_nofiltecho	FM_NOFILTECHO	/* do not echo filter operations */

/* function modifiers (|write|) */

#define	FM_CCO		(1<<(0+4))	/* cancel output cancel */
#define	FM_RAWOUT	(1<<(1+4))	/* raw output */

#define	fm_cco		FM_CCO
#define	fm_rawout	FM_RAWOUT

/* function modifiers (|status|) */

/* function modifiers (|cancel|) */

#define	FM_CANALL	(1<<(0+4))	/* cancel all requests w/ FC */
#define	FM_ALL		FM_CANALL	/* cancel all requests w/ FC */

#define	fm_canall	FM_CANALL
#define	fm_all		FM_ALL


/* external subroutines */

#if	(! defined(LIBU_MASTER)) || (LIBU_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern LONG	unixtime(LONG *) ;

extern int	u_brk(const void *) ;
extern int	u_sbrk(int,void **) ;

extern int	u_uname(struct utsname *) ;
extern int	u_unixtime(LONG *) ;
extern int	u_getloadavg(unsigned int *,int) ;
extern int	u_sysinfo(int,char *,int) ;
extern int	u_adjtime(struct timeval *,struct timeval *) ;
extern int	u_mincore(caddr_t,size_t,char *) ;
extern int	u_ulimit(int,int) ;

extern int	u_getpgid(pid_t) ;
extern int	u_getsid(pid_t) ;
extern int	u_getgroups(int,gid_t *) ;
extern int	u_setgroups(int,const gid_t *) ;
extern int	u_setuid(uid_t) ;
extern int	u_seteuid(uid_t) ;
extern int	u_setreuid(uid_t,uid_t) ;
extern int	u_setgid(gid_t) ;
extern int	u_setegid(gid_t) ;
extern int	u_setregid(gid_t,gid_t) ;
extern int	u_setpgid(pid_t,pid_t) ;
extern int	u_setsid() ;

extern int	u_sigaction(int,struct sigaction *,struct sigaction *) ;
extern int	u_sigprocmask(int,sigset_t *,sigset_t *) ;
extern int	u_sigsuspend(const sigset_t *) ;
extern int	u_sigwait(const sigset_t *,int *) ;
extern int	u_sigpending(sigset_t *) ;
extern int	u_pause() ;
extern int	u_sigaltstack(const stack_t *,stack_t *) ;

#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
extern int	u_sigsend(idtype_t,id_t,int) ;
extern int	u_sigsendset(procset_t *,int) ;
#endif

extern int	u_fork() ;
extern int	u_vfork() ;
extern int	u_getcontext(ucontext_t *) ;
extern int	u_setcontext(const ucontext_t *) ;
extern int	u_execve(const char *, const char **,const char **) ;
extern int	u_execv(const char *,const char **) ;
extern int	u_execvp(const char *,const char **) ;
extern int	u_exit(int) ;
extern int	u_kill(pid_t,int) ;
extern int	u_waitpid(pid_t,int *,int) ;
extern int	u_getrlimit(int,struct rlimit *) ;
extern int	u_setrlimit(int,const struct rlimit *) ;
extern int	u_nice(int) ;

extern int	u_mknod(const char *,mode_t,dev_t) ;
extern int	u_mkdir(const char *,mode_t) ;
extern int	u_chdir(const char *) ;
extern int	u_readlink(const char *,char *,int) ;
extern int	u_pathconf(const char *,int,long *) ;
extern int	u_statvfs(const char *,struct ustatvfs *) ;
extern int	u_stat(const char *,struct ustat *) ;
extern int	u_lstat(const char *,struct ustat *) ;
extern int	u_creat(const char *,mode_t) ;
extern int	u_open(const char *,int,mode_t) ;

#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)
extern int	u_getdents(int,dirent_t *,int) ;
#endif

#if	defined(SYSHAS_TRANS64) && (SYSHAS_TRANS64 != 0)
extern int	u_statvfs64(const char *,struct statvfs64 *) ;
extern int	u_stat64(const char *,struct stat64 *) ;
extern int	u_lstat64(const char *,struct stat64 *) ;
extern int	u_creat64(const char *,int) ;
extern int	u_open64(const char *,int,mode_t) ;
#endif

extern int	u_fchdir(int) ;
extern int	u_fsync(int) ;
extern int	u_dup(int) ;
extern int	u_dup2(int,int) ;
extern int	u_pipe(int *) ;
extern int	u_socket(int,int,int) ;
extern int	u_getsockopt(int,int,int,void *,int *) ;
extern int	u_setsockopt(int,int,int,const void *,int) ;
extern int	u_socketpair(int,int,int,int *) ;
extern int	u_connect(int,const void *,int) ;
extern int	u_accept(int,const void *,int *) ;

extern int	u_poll(struct pollfd *,int,int) ;
extern int	u_read(int,void *,int) ;
extern int	u_readv(int,struct iovec *,int) ;
extern int	u_pread(int,void *,int,offset_t) ;
extern int	u_write(int,const void *,int) ;
extern int	u_writev(int,const struct iovec *,int) ;
extern int	u_pwrite(int,const void *,int,offset_t) ;
#if	defined(SYSHAS_STREAMS) && (SYSHAS_STREAMS > 0)
extern int	u_putmsg(int,struct strbuf *,struct strbuf *,int) ;
extern int	u_putpmsg(int,struct strbuf *,struct strbuf *,int,int) ;
#endif
extern int	u_fpathconf(int,int,long *) ;
extern int	u_fstat(int,struct ustat *) ;
extern int	u_fstatvfs(int,struct ustatvfs *) ;

#if	defined(SYSHAS_TRANS64) && (SYSHAS_TRANS64 != 0)
extern int	u_fstat64(int,struct stat64 *) ;
extern int	u_fstatvfs64(int,struct statvfs64 *) ;
#endif

extern int	u_fchown(int,uid_t,gid_t) ;
extern int	u_fcntl(int,int,...) ;
extern int	u_ioctl(int,int,...) ;
extern int	u_fchmod(int,mode_t) ;
extern int	u_rewind(int) ;
extern int	u_seek(int,offset_t,int) ;
extern int	u_tell(int,offset_t *) ;
extern int	u_seeko(int,offset_t,int,offset_t *) ;

#if	defined(SYSHAS_TRANS64) && (SYSHAS_TRANS64 != 0)
extern int	u_seek64(int,off64_t,int) ;
extern int	u_tell64(int,off64_t *) ;
extern int	u_oseek64(int,off64_t,int,off64_t *) ;
extern int	u_seekoff64(int,off64_t,int,off64_t *) ;
#endif

extern int	u_bind(int,void *,int) ;
extern int	u_getsockname(int,void *,int *) ;
extern int	u_getpeername(int,void *,int *) ;
extern int	u_listen(int,int) ;
extern int	u_send(int,const void *,int,int) ;
extern int	u_sendto(int,const void *,int,int,void *,int) ;
extern int	u_sendmsg(int,struct msghdr *,int) ;
extern int	u_recv(int,void *,int,int) ;
extern int	u_recvfrom(int,void *,int,int,void *,int *) ;
extern int	u_recvmsg(int,struct msghdr *,int) ;
extern int	u_shutdown(int,int) ;
extern int	u_close(int) ;

extern int	u_resolvepath(const char *,char *,int) ;
extern int	u_access(const char *,int) ;
extern int	u_link(const char *,const char *) ;
extern int	u_unlink(const char *) ;
extern int	u_rmdir(const char *) ;
extern int	u_rename(const char *,const char *) ;
extern int	u_symlink(const char *,const char *) ;
extern int	u_chown(const char *,uid_t,gid_t) ;
extern int	u_lchown(const char *,uid_t,gid_t) ;
extern int	u_chmod(const char *,mode_t) ;
extern int	u_utime(const char *,const struct utimbuf *) ;
extern int	u_utimes(const char *,const struct timeval *) ;
extern int	u_sync() ;

extern int	u_mmap(caddr_t,size_t,int,int,int,offset_t,void *) ;
extern int	u_mprotect(const void *,size_t,int) ;
extern int	u_memcntl(const void *,size_t,int,caddr_t,int,int) ;
extern int	u_mlockall(int) ;
extern int	u_munlockall(void) ;
extern int	u_mlock(const void *,size_t) ;
extern int	u_munlock(const void *,size_t) ;
extern int	u_munmap(const void *,size_t) ;
extern int	u_plock(int) ;

#if	defined(SYSHAS_ACL) && (SYSHAS_ACL > 0)
extern int	u_acl(const char *,int,int,aclent_t *) ;
extern int	u_facl(int,int,int,aclent_t *) ;
#endif /* SYSHAS_ACL */

#ifdef	__cplusplus
}
#endif

#endif /* LIBU_MASTER */


#if	(! defined(LIBUC_MASTER)) || (LIBUC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	uc_exit(int) ;
extern int	uc_atfork(void (*)(),void (*)(),void (*)()) ;
extern int	uc_atforkrelease(void (*)(),void (*)(),void (*)()) ;
extern int	uc_atexit(void (*)()) ;
extern int	uc_ftime(struct timeb *) ;
extern int	uc_clockset(clockid_t,const struct timespec *) ;
extern int	uc_clockget(clockid_t,struct timespec *) ;
extern int	uc_clockres(clockid_t,struct timespec *) ;
extern int	uc_gethostid(unsigned int *) ;
extern int	uc_gethostname(char *,int) ;
extern int	uc_gettimeofday(struct timeval *,void *) ;
extern int	uc_getloadavg(double *,int) ;
extern int	uc_nprocessors(int) ;
extern int	uc_syspages(int) ;
extern int	uc_nprocs(int) ;

#if	SYSHAS_TIMER && (SYSHAS_TIMER > 0)
extern int uc_timercreate(clockid_t,struct sigevent *,timer_t *) ;
extern int uc_timerdestroy(timer_t) ;
extern int uc_timerdelete(timer_t) ;
extern int uc_timerset(timer_t,int,struct itimerspec *,struct itimerspec *) ;
extern int uc_timerget(timer_t,struct itimerspec *) ;
extern int uc_timerover(timer_t) ;
#endif /* SYSHAS_TIMER */

extern int	uc_fork() ;
extern int	uc_forklockbegin(int) ;
extern int	uc_forklockend() ;
extern int	uc_execve(const char *,const char **,const char **) ;
extern int	uc_isaexecve(const char *,const char **,const char **) ;
extern int	uc_initgroups(cchar *,gid_t) ;

extern int	uc_getcwd(char *,int) ;
extern int	uc_sysconf(int,long *) ;
extern int	uc_swapcontext(ucontext_t *,const ucontext_t *) ;
extern int	uc_msync(caddr_t,size_t,int) ;
extern int	uc_getauid() ;
extern int	uc_getpriority(int,id_t,int *) ;
extern int	uc_setpriority(int,id_t,int) ;
extern int	uc_getpuid(pid_t) ;
extern int	uc_procpid(cchar *,uid_t) ;

extern int	uc_createfile(const char *,mode_t) ;
extern int	uc_openpt(int) ;
extern int	uc_realpath(const char *,char *) ;
extern int	uc_truncate(const char *,offset_t) ;
extern int	uc_open(const char *,int,mode_t) ;
extern int	uc_opene(const char *,int,mode_t,int) ;
extern int	uc_openex(const char *,int,mode_t,int,int) ;
extern int	uc_openenv(const char *,int,mode_t,const char **,int) ;
extern int	uc_openinfo(struct ucopeninfo *) ;
extern int	uc_create(cchar *,mode_t) ;
extern int	uc_fpassfd(int,int) ;
extern int	uc_ftruncate(int,offset_t) ;
extern int	uc_ftat(int,struct ustat *) ;
extern int	uc_stat(const char *,struct ustat *) ;
extern int	uc_lstat(const char *,struct ustat *) ;
extern int	uc_readlink(const char *,char *,int) ;
extern int	uc_pipe2(int *,int) ;
extern int	uc_chmod(cchar *,mode_t) ;
extern int	uc_chown(cchar *,uid_t,gid_t) ;
extern int	uc_rename(cchar *,cchar *) ;
extern int	uc_utime(cchar *,const struct utimbuf *) ;

#if	defined(SYSHAS_TRANS64) && (SYSHAS_TRANS64 != 0)
extern int	uc_truncate64(const char *,off64_t) ;
extern int	uc_open64(const char *,int,mode_t) ;
extern int	uc_opene64(const char *,int,mode_t,int) ;
extern int	uc_openex64(const char *,int,mode_t,int,int) ;
extern int	uc_openenv64(const char *,int,mode_t,const char **,int) ;
extern int	uc_ftruncate64(int,off64_t) ;
extern int	uc_stat64(const char *,struct stat64 *) ;
extern int	uc_lstat64(const char *,struct stat64 *) ;
#endif

extern int	uc_isatty(int) ;
extern int	uc_fsync(int) ;
extern int	uc_fdatasync(int) ;
extern int	uc_fattach(int,const char *) ;
extern int	uc_fdetach(const char *) ;
extern int	uc_minmod(const char *,mode_t) ;
extern int	uc_fminmod(int,mode_t) ;
extern int	uc_fsize(int) ;

extern int	uc_opensocket(const char *,int,int) ;
extern int	uc_openproto(const char *,int,int,int) ;
extern int	uc_openpass(const char *,int,int,int) ;
extern int	uc_openuser(const char *,const char *,int,mode_t,int) ;
extern int	uc_openuserinfo(struct ucopeninfo *) ;
extern int	uc_openprog(const char *,int,
			const char **,const char **) ;
extern int	uc_opendialer(const char *,const char *,int,mode_t,
			const char **,const char **,int) ;
extern int	uc_openfsvc(const char *,const char *,const char *,int,mode_t,
			const char **,const char **,int) ;
extern int	uc_openusvc(const char *,const char *,const char *,int,mode_t,
			const char **,const char **,int) ;
extern int	uc_openfint(const char *,const char *,const char *,
			const char *, const char *, int,mode_t,
			const char **,const char **,int) ;
extern int	uc_opensys(const char *,int,mode_t,const char **,int,int) ;
extern int	uc_opendev(const char *,int,mode_t,const char **,int,int) ;

extern int	uc_accepte(int,const void *,int *,int) ;
extern int	uc_connecte(int,const void *,int,int) ;

extern int	uc_copy(int,int,int) ;

extern int	uc_readn(int,void *,int) ;
extern int	uc_readline(int,char *,int) ;
extern int	uc_readlinetimed(int,char *,int,int) ;

extern int	uc_write(int,const void *,int,int) ;
extern int	uc_writen(int,const void *,int) ;
extern int	uc_writedesc(int,int,int) ;
extern int	uc_writefile(int,const char *) ;

extern int	uc_reade(int,void *,int,int,int) ;
extern int	uc_recve(int,void *,int,int,int,int) ;
extern int	uc_recvfrome(int,void *,int,int,void *,int *,int,int) ;
extern int	uc_recvmsge(int,struct msghdr *,int,int,int) ;
extern int	uc_sockatmark(int) ;
extern int	uc_peek(int,void *,int) ;
extern int	uc_getsocktype(int) ;

extern int	uc_lockf(int,int,offset_t) ;
extern int	uc_lockfile(int,int,offset_t,offset_t,int) ;
extern int	uc_tcsetattr(int,int,struct termios *) ;
extern int	uc_tcgetattr(int,struct termios *) ;
extern int	uc_tcgetsid(int) ;
extern int	uc_tcsetpgrp(int,pid_t) ;
extern int	uc_tcgetpgrp(int) ;
extern int	uc_keepalive(int,int) ;
extern int	uc_reuseaddr(int) ;
extern int	uc_moveup(int,int) ;
extern int	uc_ndelay(int,int) ;
extern int	uc_nonblock(int,int) ;
extern int	uc_msgdiscard(int) ;
extern int	uc_setappend(int,int) ;
extern int	uc_closeonexec(int,int) ;
extern int	uc_close(int) ;

extern int	uc_remove(const char *) ;
extern int	uc_link(const char *,const char *) ;
extern int	uc_symlink(const char *,const char *) ;
extern int	uc_unlink(const char *) ;
extern int	uc_lockend(int,int,int,int) ;
extern int	uc_mkdir(const char *,mode_t) ;
extern int	uc_rmdir(const char *) ;
extern int	uc_access(const char *,int) ;
extern int	uc_pathconf(const char *,int,long *) ;

extern int	uc_malloc(int,void *) ;
extern int	uc_calloc(int,int,void *) ;
extern int	uc_valloc(int,void *) ;
extern int	uc_realloc(const void *,int,void *) ;
extern int	uc_free(const void *) ;
extern int	uc_mallset(int) ;
extern int	uc_mallout(unsigned int *) ;
extern int	uc_mallinfo(unsigned int *,int) ;
extern int	uc_mallpresent(const void *) ;
extern int	uc_mallocstrw(const char *,int,const char **) ;
extern int	uc_mallocbuf(const char *,int,const void **) ;
extern int	uc_libmallocstrw(cchar *,int,cchar **) ;
extern int	uc_libmalloc(int,void *) ;
extern int	uc_libvalloc(int,void *) ;
extern int	uc_librealloc(const void *,int,void *) ;
extern int	uc_libfree(const void *) ;

extern int	uc_raise(int) ;
extern int	uc_sigdefault(int) ;
extern int	uc_sigignore(int) ;
extern int	uc_sighold(int) ;
extern int	uc_sigrelease(int) ;
extern int	uc_sigpause(int) ;
extern int	uc_sigqueue(pid_t,int,const union sigval) ;
extern int	uc_sigwaitinfo(const sigset_t *,siginfo_t *) ;
extern int	uc_sigtimedwait(const sigset_t *,siginfo_t *,
			const struct timespec *) ;
extern int	uc_sigwaitinfoto(const sigset_t *,siginfo_t *,
			const struct timespec *) ;
extern int	uc_sigsetempty(sigset_t *) ;
extern int	uc_sigsetfill(sigset_t *) ;
extern int	uc_sigsetadd(sigset_t *,int) ;
extern int	uc_sigsetdel(sigset_t *,int) ;
extern int	uc_sigsetismem(sigset_t *,int) ;

extern int	uc_safesleep(int) ;
extern int	uc_mktime(struct tm *,time_t *) ;
extern int	uc_gmtime(const time_t *,struct tm *) ;
extern int	uc_localtime(const time_t *,struct tm *) ;
extern int	uc_ttyname(int,char *,int) ;
extern int	uc_mkfifo(const char *,mode_t) ;
extern int	uc_piper(int *,int) ;

extern int	uc_plock(int) ;
extern int	uc_mlock(const void *,size_t) ;
extern int	uc_munlock(const void *,size_t) ;
extern int	uc_mlockall(int) ;
extern int	uc_munlockall() ;
extern int	uc_madvise(const void *,size_t,int) ;

extern int	uc_getrandom(void *,int,int) ;
extern int	uc_getentropy(void *,int) ;

/* string-digit conversion */

extern int	uc_strtoi(const char *,char **,int,int *) ;
extern int	uc_strtol(const char *,char **,int,long *) ;
extern int	uc_strtoll(const char *,char **,int,LONG *) ;
extern int	uc_strtoui(const char *,char **,int,unsigned int *) ;
extern int	uc_strtoul(const char *,char **,int,unsigned long *) ;
extern int	uc_strtoull(const char *,char **,int,unsigned long long *) ;

/* OS PASSWD database */
extern int	uc_setpwent() ;
extern int	uc_getpwent(struct passwd *,char *,int) ;
extern int	uc_getpwnam(const char *,struct passwd *,char *,int) ;
extern int	uc_getpwuid(uid_t,struct passwd *,char *,int) ;
extern int	uc_endpwent() ;

/* OS GROUP database */
extern int	uc_setgrent() ;
extern int	uc_getgrent(struct group *,char *,int) ;
extern int	uc_getgrnam(const char *,struct group *,char *,int) ;
extern int	uc_getgrgid(gid_t,struct group *,char *,int) ;
extern int	uc_endgrent() ;

#if	defined(SYSHAS_SHADOW) && (SYSHAS_SHADOW > 0)

/* OS SHADOW database */
extern int	uc_setspent() ;
extern int	uc_getspent(struct spwd *,char *,int) ;
extern int	uc_getspnam(const char *,struct spwd *,char *,int) ;
extern int	uc_endspent() ;

#endif /* SYSHAS_SHADOW */

#if	defined(SYSHAS_PROJECT) && (SYSHAS_PROJECT > 0)

/* OS PROJECT database */
extern int	uc_setpjent() ;
extern int	uc_getpjent(struct project *,char *,int) ;
extern int	uc_getpjbyid(projid_t,struct project *,char *,int) ;
extern int	uc_getpjbyname(const char *,struct project *,char *,int) ;
extern int	uc_endpjent() ;
extern int	uc_getdefaultproj(const char *,struct project *,char *,int) ;
extern int	uc_inproj(const char *,const char *,char *,int) ;

#endif /* SYSHAS_PROJECT */

#if	defined(SYSHAS_USERATTR) && (SYSHAS_USERATTR > 0)

/* OS USERATTR database (different from the others) */
extern int	uc_setuserattr() ;
extern int	uc_getuserattr(userattr_t **) ;
extern int	uc_getusernam(const char *,userattr_t **) ;
extern int	uc_getuseruid(uid_t,userattr_t **) ;
extern int	uc_freeuserattr(userattr_t *) ;
extern int	uc_enduserattr() ;
extern int	uc_kvamatch(kva_t *,const char *,const char **) ;

#endif /* SYSHAS_USERATTR */

/* POSIX shared memory operations */
extern int	uc_openshm(const char *,int,mode_t) ;
extern int	uc_openshmto(const char *,int,mode_t,int) ;
extern int	uc_unlinkshm(const char *) ;

/* miscellaneous */
extern int	uc_inetpton(int,const char *,void *) ;
extern int	uc_inetntop(int,const void *,char *,int) ;
extern int	uc_confstr(int,char *,int) ;
extern int	uc_strtod(const char *,char **,double *) ;
extern int	uc_getnetname(char *) ;

/* NETWORK PROTOCOL database */

extern int	uc_setprotoent(int) ;
extern int	uc_endprotoent() ;
extern int	uc_getprotoent(struct protoent *,char *,int) ;
extern int	uc_getprotobyname(const char *,struct protoent *,char *,int) ;
extern int	uc_getprotobynumber(int,struct protoent *,char *,int) ;

/* NETWORK NET database */

extern int	uc_setnetent(int) ;
extern int	uc_endnetent() ;
extern int	uc_getnetent(struct netent *,char *,int) ;
extern int	uc_getnetbyname(const char *,
			struct netent *,char *,int) ;
extern int	uc_getnetbyaddr(long,int,
			struct netent *,char *,int) ;

/* NETWORK HOST database */

extern int	uc_sethostent(int) ;
extern int	uc_endhostent() ;
extern int	uc_gethostent(struct hostent *,char *,int) ;
extern int	uc_gethostbyname(const char *,
			struct hostent *,char *,int) ;
extern int	uc_gethostbyaddr(const char *,int,int,
			struct hostent *,char *,int) ;

/* NETWORK SERVICE database */

extern int	uc_setservent(int) ;
extern int	uc_endservent() ;
extern int	uc_getservent(struct servent *,char *,int) ;
extern int	uc_getservbyname(const char *,const char *,
			struct servent *,char *,int) ;
extern int	uc_getservbyport(int,const char *,
			struct servent *,char *,int) ;

/* NETWORK IPNODE database */

extern int	uc_getipnodebyname(const char *,int,int,struct hostent **) ;
extern int	uc_freehostent(struct hostent *) ;

/* NETWORK ADDRINFO database */

extern int	uc_getaddrinfo(const char *,const char *,
			struct addrinfo *,struct addrinfo **) ;
extern int	uc_freeaddrinfo(struct addrinfo *) ;

/* NETWORK SOCKADDR combined database */

extern int	uc_getnameinfo(const struct sockaddr *,int,
			char *,int,char *,int,int) ;

/* USER-SHELL database */

extern int	uc_setus() ;
extern int	uc_getus(char *,int) ;
extern int	uc_endus() ;

/* Solaris® Threads */

#if	defined(OSNAME_SunOS) && (OSNAME_SunOS > 0)

extern int uc_thrcreate(caddr_t,size_t,int (*)(void *),const void *,long) ;
extern int uc_threxit(int) ;
extern int uc_thrjoin(thread_t,int *) ;
extern int uc_thrsuspend(thread_t) ;
extern int uc_thrcontinue(thread_t) ;
extern int uc_thrminstack() ;
extern int uc_thrkill(thread_t,int) ;
extern int uc_thrmain() ;
extern int uc_thrself() ;
extern int uc_thryield() ;
extern int uc_thrsigsetmask(int,const sigset_t *,sigset_t *) ;
extern int uc_thrstksegment(stack_t *) ;
extern int uc_thrkeycreate(thread_key_t *,void (*)(void *)) ;
extern int uc_thrsetspecific(thread_key_t,void *) ;
extern int uc_thrgetspecific(thread_key_t,void **) ;
extern int uc_thrgetconcurrency() ;
extern int uc_thrsetconcurrency(int) ;

#endif /* Solaris® Threads */

#ifdef	__cplusplus
}
#endif

#endif /* LIBUC_MASTER */


/* POSIX threads stuff */

#if	defined(PTHREAD) && (PTHREAD > 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	pt_sigmask(int,sigset_t *,sigset_t *) ;
extern int	pt_atfork(void (*)(),void (*)(),void (*)()) ;

#ifdef	__cplusplus
}
#endif

#endif /* defined(PTHREAD) && (PTHREAD > 0) */


#if	defined(SYSHAS_XTI) && (SYSHAS_XTI > 0)
#if	(! defined(LIBUT_MASTER)) || (LIBUT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	ut_alloc(int,int,int,void **) ;
extern int	ut_open(const char *,int,struct t_info *) ;
extern int	ut_bind(int,struct t_bind *,struct t_bind *) ;
extern int	ut_listen(int,struct t_call *) ;
extern int	ut_connect(int,struct t_call *,struct t_call *) ;
extern int	ut_accept(int,int,const struct t_call *) ;
extern int	ut_look(int) ;
extern int	ut_sync(int) ;
extern int	ut_close(int) ;
extern int	ut_free(void *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* LIBUT_MASTER */
#endif /* defined(SYSHAS_XTI) && (SYSHAS_XTI > 0) */


/* other */

#if	(! defined(VSYSTEM_MASTER)) || (VSYSTEM_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	vs_intenable(sigset_t *) ;
extern int	vs_intdisable(sigset_t *) ;

#ifdef	__cplusplus
}
#endif

#endif /* (! defined(VSYSTEM_MASTER)) || (VSYSTEM_MASTER == 0) */

#endif /* VSYSTEM_INCLUDE */



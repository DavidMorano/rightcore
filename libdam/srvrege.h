/* srvrege */

/* machine status entry */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SRVREGE_INCLUDE
#define	SRVREGE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<netdb.h>

#include	<sockaddress.h>
#include	<localmisc.h>


/* object defines */

#define	SRVREGE_ALL		struct srvrege_all
#define	SRVREGE_UTIME		struct srvrege_utime


/* the interface types for users */

#define	SRVREGE_TEMPTY		0	/* entry is unused */
#define	SRVREGE_TDISABLED	1	/* currently disabled (not serving) */
#define	SRVREGE_TFIFO		2	/* FIFO file */
#define	SRVREGE_TPIPE		3	/* pipe file (NAMEFS or not) */
#define	SRVREGE_TPASSFD		4	/* pass FD (to FIFO or PIPE) */
#define	SRVREGE_TFMQ		5	/* file message queue */
#define	SRVREGE_TTLISTREAMU	6	/* TLI stream unordered release */
#define	SRVREGE_TTLISTREAMO	7	/* TLI stream ordered release */
#define	SRVREGE_TTLIDGRAM	8	/* TLI data-gram */
#define	SRVREGE_TSOCKSTREAM	9	/* socket STREAM */
#define	SRVREGE_TSOCKDGRAM	10	/* socket DGRAM */
#define	SRVREGE_TPMQ		11	/* POSIX message queue */


/* other (for user) */

#define	SRVREGE_TAGLEN	14


/* the rest of these defines are for internal usage (mostly I think) */


/* entry field lengths */
#define	SRVREGE_LSTIME		4
#define	SRVREGE_LUTIME		4
#define	SRVREGE_LHOSTID		4
#define	SRVREGE_LITYPE		4
#define	SRVREGE_LPF		4
#define	SRVREGE_LPTYPE		4
#define	SRVREGE_LPROTO		4
#define	SRVREGE_LA		sizeof(union srvrege_addr)
#define	SRVREGE_LTAG		SRVREGE_TAGLEN
#define	SRVREGE_LSVC		MAXNAMELEN
#define	SRVREGE_LSS		MAXNAMELEN
#define	SRVREGE_LHOST		MAXHOSTNAMELEN


/* entry field offsets */
/* do this carefully ! */
/* there is no good automatic way to do this in C language (sigh) */
/* the C language does not have all of the advantages of assembly language */

#define	SRVREGE_OUTIME		(0 * 4)
#define	SRVREGE_OSTIME		(1 * 4)
#define	SRVREGE_OHOSTID		(2 * 4)
#define	SRVREGE_OITYPE		(3 * 4)
#define	SRVREGE_OPF		(4 * 4)
#define	SRVREGE_OPTYPE		(5 * 4)
#define	SRVREGE_OPROTO		(6 * 4)
#define	SRVREGE_OPID		(7 * 4)
#define	SRVREGE_OA		(8 * 4)
#define	SRVREGE_OTAG		(SRVREGE_OA + SRVREGE_LA)
#define	SRVREGE_OSVC		(SRVREGE_OTAG + SRVREGE_LTAG)
#define	SRVREGE_OSS		(SRVREGE_OSVC + SRVREGE_LSVC)
#define	SRVREGE_OHOST		(SRVREGE_OSS + SRVREGE_LSS)

#define	SRVREGE_SIZE		(SRVREGE_OHOST + SRVREGE_LHOST)


union srvrege_addr {
	SOCKADDRESS	sa ;			/* socket address */
	char		fp[MAXPATHLEN + 1] ;	/* file path */
	char		nb[MAXPATHLEN + 1] ;	/* net-addr buffer */
} ;

struct srvrege_all {
	uint	utime ;			/* update time */
	uint	stime ;			/* starting time */
	uint	hostid ;		/* host ID */
	uint	itype ;			/* interface type */
	uint	pf ;			/* protocol family (for sockets) */
	uint	ptype ;			/* protocol type */
	uint	proto ;			/* protocol number (if used) */
	uint	pid ;			/* PID of server */
	union	srvrege_addr	a ;
	char	tag[SRVREGE_TAGLEN + 1] ;	/* service tag */
	char	svc[MAXNAMELEN + 1] ;	/* service name */
	char	ss[MAXNAMELEN + 1] ;	/* subservice name */
	char	host[MAXHOSTNAMELEN + 1] ;
} ;

struct srvrege_utime {
	uint	utime ;
} ;


#if	(! defined(SRVREGE_MASTER)) || (SRVREGE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int srvrege_all(char *,int,int,struct srvrege_all *) ;
extern int srvrege_utime(char *,int,int,struct srvrege_utime *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SRVREGE_MASTER */

#endif /* SRVREGE_INCLUDE */



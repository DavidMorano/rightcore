/* sysmisc */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSMISC_INCLUDE
#define	SYSMISC_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<netdb.h>

#include	<localmisc.h>


#ifndef	SYSMISC_PROVIDERLEN
#ifdef	PROVIDERLEN
#define	SYSMISC_PROVIDERLEN	PROVIDERLEN
#else
#define	SYSMISC_PROVIDERLEN	100
#endif
#endif


/* client request message */
struct sysmisc_request {
	uint	msglen ;
	uint	tag ;
	uint	duration ;		/* turns on repetition (secs) */
	uint	interval ;		/* repetition interval (secs) */
	ushort	addrfamily ;		/* used for UDP response */
	ushort	addrport ;		/* used for UDP response */
	uint	addrhost[4] ;		/* used for UDP response */
	ushort	opts ;			/* request options */
	uchar	msgtype ;		/* message type */
} ;

/* server response 1 */
struct sysmisc_loadave {
	uint	msglen ;
	uint	tag ;
	uint	timestamp ;
	uint	providerid ;
	uint	hostid ;
	uint	la_1min ;
	uint	la_5min ;
	uint	la_15min ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
} ;

/* server response 2 */
struct sysmisc_extra {
	uint	msglen ;
	uint	tag ;
	uint	timestamp ;
	uint	providerid ;
	uint	hostid ;
	uint	la_1min ;
	uint	la_5min ;
	uint	la_15min ;
	uint	boottime ;
	uint	nproc ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
} ;

/* server response 3 */
struct sysmisc_hostinfo {
	uint	msglen ;
	uint	tag ;
	uint	timestamp ;
	uint	providerid ;
	uint	hostid ;
	uint	la_1min ;
	uint	la_5min ;
	uint	la_15min ;
	uint	boottime ;
	uint	nproc ;
	ushort	hostnamelen ;
	uchar	msgtype ;		/* message type */
	uchar	rc ;
	char	provider[SYSMISC_PROVIDERLEN + 1] ;
	char	hostname[MAXHOSTNAMELEN + 1] ;
} ;


/* request types */
enum sysmisctypes {
	sysmisctype_request,
	sysmisctype_loadave,
	sysmisctype_extra,
	sysmisctype_hostinfo,
	sysmisctype_overlast
} ;


/* response codes */
enum sysmiscrcs {
	sysmiscrc_ok,
	sysmiscrc_invalid,
	sysmiscrc_notavail,
	sysmiscrc_done,
	sysmiscrc_goingdown,
	sysmiscrc_overlast
} ;


/* message sizes */

#define	SYSMISC_SREQUEST	((7 * sizeof(uint)) + (3 * sizeof(ushort)) + 1)
#define	SYSMISC_SLOADAVE	((7 * sizeof(uint)) + (0 * sizeof(ushort)) + 2)
#define	SYSMISC_SEXTRA		((9 * sizeof(uint)) + (0 * sizeof(ushort)) + 2)
#define	SYSMISC_SHOSTINFO	((9 * sizeof(uint)) + (1 * sizeof(ushort)) + \
					MAXHOSTNAMELEN + 2)

/* options */

#define	SYSMISC_MBLANK		0x00
#define	SYSMISC_MTCP		0x01		/* also the default */
#define	SYSMISC_MUDP		0x02
#define	SYSMISC_MLOADAVE	0x04		/* load average */
#define	SYSMISC_MEXTRA		0x08		/* all information */


#if	(! defined(SYSMISC_MASTER)) || (SYSMISC_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	sysmisc_request(struct sysmisc_request *,int,char *,int) ;
extern int	sysmisc_loadave(struct sysmisc_loadave *,int,char *,int) ;
extern int	sysmisc_extra(struct sysmisc_extra *,int,char *,int) ;
extern int	sysmisc_hostinfo(struct sysmisc_hostinfo *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSMISC_MASTER */

#endif /* SYSMISC_INCLUDE */



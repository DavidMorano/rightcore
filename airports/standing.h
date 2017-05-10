/* standing */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */


#ifndef	STANDING_INCLUDE
#define	STANDING_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<arpa/inet.h>
#include	<netdb.h>

#include	<msfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local object defines */

#define	STANDING		struct standing_head
#define	STANDING_SYSMISC	struct standing_sysmisc


/* request return codes */

#define	STANDING_ROK		0
#define	STANDING_RUNDEFINED	1
#define	STANDING_RINVALID	2
#define	STANDING_RERROR		3


struct standing_sysmisc {
	time_t		boottime ;
	uint		ncpu ;
	uint		nproc ;
	uint		la_1min, la_5min, la_15min ;
} ;

struct standing_afsocket {
	time_t		lastaccess ;
	int		fd ;
} ;

struct standing_ms {
	MSFILE_ENT	e ;
	time_t		ti_la ;			/* last update */
	time_t		ti_numbers ;		/* last update */
	time_t		ti_checkspeed ;		/* last update */
	caddr_t		shm ;			/* the buffer */
	char		*shmfname ;
	uint		pagesize ;
	int		pid ;			/* outstanding PID */
} ;

struct standing_cache {
	STANDING_SYSMISC	d ;
	time_t		ti_access ;		/* last access */
	time_t		ti_sysmisc ;		/* last SYSMISC update */
	time_t		ti_loadave ;		/* last LOADAVE update */
} ;

struct standing_callout {
	SOCKADDRESS	sa ;
	time_t		next, expire ;
	uint		tag ;
	uint		duration ;
	uint		interval ;
	int		af ;
	int		salen ;
	int		type ;
} ;

struct standing_flags {
	uint		kopen:1 ;
	uint		interval:1 ;
	uint		kiopen:1 ;
	uint		msopen:1  ;
	uint		speedout:1 ;
} ;

struct standing_head {
	struct proginfo		*pip ;
	struct standing_flags	f ;
	struct standing_cache	c ;
	struct standing_ms	m ;
	struct standing_afsocket	afs_unix, afs_inet4, afs_inet6 ;
	MSFILE			ms ;
	VECITEM			callouts ;
	int			nactive ;
	int			mininterval ;
} ;


#if	(! defined(STANDING_MASTER)) || (STANDING_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int standing_start(STANDING *,struct proginfo *) ;
extern int standing_finish(STANDING *) ;
extern int standing_request(STANDING *,time_t,int,char *,char *) ;
extern int standing_check(STANDING *,time_t) ;
extern int standing_readdata(STANDING *,STANDING_SYSMISC *) ;

#ifdef	__cplusplus
}
#endif

#endif /* STANDING_MASTER */


#endif /* STANDING_INCLUDE */




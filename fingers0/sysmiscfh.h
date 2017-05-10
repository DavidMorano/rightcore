/* sysmiscfh */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSMISCFH_INCLUDE
#define	SYSMISCFH_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#define	SYSMISCFH		struct sysmiscfh

#define	SYSMISCFH_MAGICSTR	"SYSMISC"
#define	SYSMISCFH_MAGICSIZE	16
#define	SYSMISCFH_VERSION	0
#define	SYSMISCFH_IDLEN		20


enum sysmiscfvs {
	sysmiscfv_shmsize,
	sysmiscfv_intstale,
	sysmiscfv_utime,
	sysmiscfv_btime,
	sysmiscfv_ncpu,
	sysmiscfv_nproc,
	sysmiscfv_la,
	sysmiscfv_overlast
} ;

struct sysmiscfh {
	uint		shmsize ;
	uint		intstale ;
	uint		utime ;
	uint		btime ;
	uint		ncpu ;
	uint		nproc ;
	uint		la[3] ;
	uchar		vetu[4] ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int sysmiscfh(SYSMISCFH *,int,char *,int) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSMISCFH_INCLUDE */



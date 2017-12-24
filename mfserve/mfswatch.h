/* mfs-watch */

/* last modified %G% version %I% */


/* revision history:

	= 2004-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSERVE.

*/

/* Copyright © 2004,2017 David A­D­ Morano.  All rights reserved. */

#ifndef	MFSWATCH_INCLUDE
#define	MFSWATCH_INCLUDE	1

#include	<envstandards.h>
#include	<sys/types.h>
#include	<poller.h>
#include	<svcfile.h>
#include	<envhelp.h>
#include	<vecpstr.h>
#include	<osetstr.h>

#include	"sreqdb.h"
#include	"mfsmain.h"
#include	"mfsbuilt.h"
#include	"defs.h"


#define	MFSWATCH		struct mfswatch
#define	MFSWATCH_FL		struct mfswatch_flags


struct mfswatch_flags {
	uint		done:1 ;	/* exit ASAP */
	uint		built:1 ;
	uint		svcfile:1 ;
	uint		eh:1 ;		/* environment-mangement helper */
	uint		bindirs:1 ;	/* path-executable */
	uint		libdirs:1 ;	/* path-library */
	uint		users:1 ;
} ;

struct mfswatch {
	MFSWATCH_FL	f, open ;
	SREQDB		reqs ;		/* service requests */
	POLLER		pm ;		/* Poll-Manager */
	MFSBUILT	built ;		/* builtin lookup database */
	SVCFILE		tabs ;		/* service lookup database */
	ENVHELP		eh ;		/* environment-management helper */
	vecpstr		bindirs ;	/* path-executable */
	vecpstr		libdirs ;	/* path-library */
	osetstr		users ;
	time_t		ti_mark ;
	time_t		ti_maint ;
	time_t		ti_config ;
	time_t		ti_users ;
	int		nprocs ;
	int		nthrs ;
	int		nbuilts ;
	int		pollto ;	/* poll interval in milliseconds */
} ;


enum jobtypes {
	jobtype_req,
	jobtype_listen,
	jobtype_overlast
} ;

#ifdef	__cplusplus
extern "C" {
#endif

extern int	mfswatch_begin(PROGINFO *) ;
extern int	mfswatch_service(PROGINFO *) ;
extern int	mfswatch_newjob(PROGINFO *,int,int,int,int) ;
extern int	mfswatch_end(PROGINFO *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MFSWATCH_INCLUDE */



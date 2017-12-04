/* mfs-config */

/* last modified %G% version %I% */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSERVE.

*/

/* Copyright © 2000,2017 David A­D­ Morano.  All rights reserved. */

#ifndef	MFSCONFIG_INCLUDE
#define	MFSCONFIG_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<msfile.h>
#include	<paramfile.h>
#include	<expcook.h>

#include	"mfsmain.h"
#include	"defs.h"		/* for PROGINFO */


#define	CONFIG		struct config
#define	CONFIG_FL	struct config_flags


struct config_flags {
	uint		p:1 ;
	uint		lockinfo:1 ;
} ;

struct config {
	PROGINFO	*pip ;
	CONFIG_FL	f ;
	PARAMFILE	p ;
	time_t		ti_lastcheck ;
	int		intcheck ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	config_start(CONFIG *,PROGINFO *,cchar *,int) ;
extern int	config_check(CONFIG *) ;
extern int	config_read(CONFIG *) ;
extern int	config_finish(CONFIG *) ;

#ifdef	__cplusplus
}
#endif

#endif /* MFSCONFIG_INCLUDE */



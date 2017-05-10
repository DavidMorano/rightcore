/* pcs-config */


/* revision history:

	= 2000-05-14, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 2000 David A­D­ Morano.  All rights reserved. */

#ifndef	PCSCONFIG_INCLUDE
#define	PCSCONFIG_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<msfile.h>
#include	<paramfile.h>
#include	<expcook.h>

#include	"pcsmain.h"
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
	EXPCOOK		cooks ;
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

#endif /* PCSCONFIG_INCLUDE */



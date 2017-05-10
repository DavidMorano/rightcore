/* clientinfo */


/* Copyright © 1999,2008 David A­D­ Morano.  All rights reserved. */


#ifndef	CLIENTINFO_INCLUDE
#define	CLIENTINFO_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<localmisc.h>


#ifdef	__cplusplus
extern "C" {
#endif

extern int clientinfo_start(struct clientinfo *) ;
extern int clientinfo_finish(struct clientinfo *) ;
extern int clientinfo_loadnames(struct clientinfo *,const char *) ;

#ifdef	__cplusplus
}
#endif


#endif /* CLIENTINFO_INCLUDE */



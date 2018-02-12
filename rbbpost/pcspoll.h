/* pcspoll */


/* revision history:

	= 1998-04-03, David A­D­ Morano
	This module was originally written.

	- 2008-10-07, David A­D­ Morano
        This module was modified to allow for the main part of it to be a
        loadable module.

*/

/* Copyright © 1998,2008 David A­D­ Morano.  All rights reserved. */

#ifndef	PCSPOLL_INCLUDE
#define	PCSPOLL_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>

#include	<modload.h>
#include	<pcsconf.h>
#include	<localmisc.h>

#include	"pcspolls.h"


#define	PCSPOLL		struct pcspoll_head
#define	PCSPOLL_INFO	struct pcspoll_i
#define	PCSPOLL_CA	struct pcspoll_calls
#define	PCSPOLL_FL	struct pcspoll_flags
#define	PCSPOLL_MAGIC	0x97677246


struct pcspoll_calls {
	int	(*start)(void *,PCSCONF *,const char *) ;
	int	(*info)(void *,PCSPOLLS_INFO *) ;
	int	(*cmd)(void *,int) ;
	int	(*finish)(void *) ;
} ;

struct pcspoll_i {
	int		dummy ;
} ;

struct pcspoll_flags {
	uint		loaded:1 ;
} ;

struct pcspoll_head {
	uint		magic ;
	MODLOAD		loader ;
	PCSPOLL_CA	call ;
	PCSPOLL_FL	f ;
	void		*obj ;		/* object pointer */
} ;


#if	(! defined(PCSPOLL_MASTER)) || (PCSPOLL_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	pcspoll_start(PCSPOLL *,PCSCONF *,cchar *) ;
extern int	pcspoll_info(PCSPOLL *,PCSPOLL_INFO *) ;
extern int	pcspoll_cmd(PCSPOLL *,int) ;
extern int	pcspoll_finish(PCSPOLL *) ;

#ifdef	__cplusplus
}
#endif

#endif /* PCSPOLL_MASTER */

#endif /* PCSPOLL_INCLUDE */



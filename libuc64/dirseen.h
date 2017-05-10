/* dirseen */


/* revision history:

	= 2006-09-10, David A­D­ Morano
	I created this from hacking something that was similar that was
	originally created for a PCS program.

*/

/* Copyright © 2006 David A­D­ Morano.  All rights reserved. */

#ifndef	DIRSEEN_INCLUDE
#define	DIRSEEN_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<localmisc.h>


#define	DIRSEEN		struct dirseen_head
#define	DIRSEEN_CUR	struct dirseen_c
#define	DIRSEEN_MAGIC	0x09854123
#define	DIRSEEN_NDEF	10


struct dirseen_c {
	int		i ;
} ;

struct dirseen_head {
	uint		magic ;
	VECOBJ		list ;
	int		strsize ;
} ;


#if	(! defined(DIRSEEN_MASTER)) || (DIRSEEN_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int dirseen_start(DIRSEEN *) ;
extern int dirseen_add(DIRSEEN *,const char *,int,struct ustat *) ;
extern int dirseen_havename(DIRSEEN *,const char *,int) ;
extern int dirseen_havedevino(DIRSEEN *,struct ustat *) ;
extern int dirseen_count(DIRSEEN *) ;
extern int dirseen_curbegin(DIRSEEN *,DIRSEEN_CUR *) ;
extern int dirseen_curend(DIRSEEN *,DIRSEEN_CUR *) ;
extern int dirseen_enum(DIRSEEN *,DIRSEEN_CUR *,char *,int) ;
extern int dirseen_finish(DIRSEEN *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DIRSEEN_MASTER */

#endif /* DIRSEEN_INCLUDE */



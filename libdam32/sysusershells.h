/* sysusershells */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSUSERSHELLS_INCLUDE
#define	SYSUSERSHELLS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<pwd.h>

#include	<filemap.h>
#include	<localmisc.h>


/* object defines */

#define	SYSUSERSHELLS		struct sysusershells_head
#define	SYSUSERSHELLS_MAGIC	0x88776229
#define	SYSUSERSHELLS_FNAME	"/sys/shells"


struct sysusershells_head {
	uint		magic ;
	FILEMAP		b ;
} ;


typedef struct sysusershells_head	sysusershells ;


#if	(! defined(SYSUSERSHELLS_MASTER)) || (SYSUSERSHELLS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sysusershells_open(SYSUSERSHELLS *,const char *) ;
extern int sysusershells_close(SYSUSERSHELLS *) ;
extern int sysusershells_readent(SYSUSERSHELLS *,char *,int) ;
extern int sysusershells_reset(SYSUSERSHELLS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSUSERSHELLS_MASTER */

#endif /* SYSUSERSHELLS_INCLUDE */



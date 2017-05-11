/* sysprojects */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSPROJECTS_INCLUDE
#define	SYSPROJECTS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<project.h>

#include	<filemap.h>
#include	<localmisc.h>


/* object defines */

#define	SYSPROJECTS		struct sysprojects_head
#define	SYSPROJECTS_MAGIC	0x88776216
#define	SYSPROJECTS_FNAME	"/sys/projects"


struct sysprojects_head {
	uint		magic ;
	FILEMAP		b ;
} ;


typedef struct sysprojects_head	sysprojects ;


#if	(! defined(SYSPROJECTS_MASTER)) || (SYSPROJECTS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sysprojects_open(SYSPROJECTS *,const char *) ;
extern int sysprojects_close(SYSPROJECTS *) ;
extern int sysprojects_readent(SYSPROJECTS *,struct project *,char *,int) ;
extern int sysprojects_reset(SYSPROJECTS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSPROJECTS_MASTER */

#endif /* SYSPROJECTS_INCLUDE */



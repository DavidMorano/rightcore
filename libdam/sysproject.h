/* sysproject */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSPROJECT_INCLUDE
#define	SYSPROJECT_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<project.h>

#include	<filemap.h>
#include	<localmisc.h>


/* object defines */

#define	SYSPROJECT		struct sysproject_head
#define	SYSPROJECT_MAGIC	0x88776216
#define	SYSPROJECT_FNAME	"/sys/project"


struct sysproject_head {
	uint		magic ;
	FILEMAP		b ;
} ;


typedef struct sysproject_head	sysproject ;


#if	(! defined(SYSPROJECT_MASTER)) || (SYSPROJECT_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sysproject_open(SYSPROJECT *,const char *) ;
extern int sysproject_close(SYSPROJECT *) ;
extern int sysproject_readent(SYSPROJECT *,struct project *,char *,int) ;
extern int sysproject_reset(SYSPROJECT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSPROJECT_MASTER */

#endif /* SYSPROJECT_INCLUDE */



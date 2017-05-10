/* sysgroups */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSGROUPS_INCLUDE
#define	SYSGROUPS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<grp.h>

#include	<filemap.h>
#include	<localmisc.h>


/* object defines */

#define	SYSGROUPS		struct sysgroups_head
#define	SYSGROUPS_MAGIC		0x88776215
#define	SYSGROUPS_FNAME		"/sys/groups"


struct sysgroups_head {
	uint		magic ;
	FILEMAP		b ;
} ;


typedef struct sysgroups_head	sysgroups ;


#if	(! defined(SYSGROUPS_MASTER)) || (SYSGROUPS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sysgroups_open(SYSGROUPS *,const char *) ;
extern int sysgroups_close(SYSGROUPS *) ;
extern int sysgroups_readent(SYSGROUPS *,struct group *,char *,int) ;
extern int sysgroups_reset(SYSGROUPS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSGROUPS_MASTER */

#endif /* SYSGROUPS_INCLUDE */



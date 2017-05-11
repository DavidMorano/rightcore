/* sysgroup */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSGROUP_INCLUDE
#define	SYSGROUP_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<grp.h>

#include	<filemap.h>
#include	<localmisc.h>


/* object defines */

#define	SYSGROUP		struct sysgroup_head
#define	SYSGROUP_MAGIC		0x88776216
#define	SYSGROUP_FNAME		"/sys/group"


struct sysgroup_head {
	uint		magic ;
	FILEMAP		b ;
} ;


typedef struct sysgroup_head	sysgroup ;


#if	(! defined(SYSGROUP_MASTER)) || (SYSGROUP_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sysgroup_open(SYSGROUP *,const char *) ;
extern int sysgroup_close(SYSGROUP *) ;
extern int sysgroup_readent(SYSGROUP *,struct group *,char *,int) ;
extern int sysgroup_reset(SYSGROUP *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSGROUP_MASTER */

#endif /* SYSGROUP_INCLUDE */



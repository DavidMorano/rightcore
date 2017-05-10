/* sysusers */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSUSERS_INCLUDE
#define	SYSUSERS_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<pwd.h>

#include	<filemap.h>
#include	<localmisc.h>


/* object defines */

#define	SYSUSERS		struct sysusers_head
#define	SYSUSERS_MAGIC		0x88776217
#define	SYSUSERS_FNAME		"/sys/users"


struct sysusers_head {
	uint		magic ;
	FILEMAP		b ;
} ;


typedef struct sysusers_head	sysusers ;


#if	(! defined(SYSUSERS_MASTER)) || (SYSUSERS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int sysusers_open(SYSUSERS *,const char *) ;
extern int sysusers_close(SYSUSERS *) ;
extern int sysusers_readent(SYSUSERS *,struct passwd *,char *,int) ;
extern int sysusers_reset(SYSUSERS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSUSERS_MASTER */

#endif /* SYSUSERS_INCLUDE */



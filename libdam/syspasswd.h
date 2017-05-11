/* syspasswd */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSPASSWD_INCLUDE
#define	SYSPASSWD_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<pwd.h>

#include	<filemap.h>
#include	<localmisc.h>


/* object defines */

#define	SYSPASSWD		struct syspasswd_head
#define	SYSPASSWD_MAGIC		0x88776281
#define	SYSPASSWD_FNAME		"/sys/passwd"


struct syspasswd_head {
	uint		magic ;
	FILEMAP		b ;
} ;


typedef struct syspasswd_head	syspasswd ;


#if	(! defined(SYSPASSWD_MASTER)) || (SYSPASSWD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int syspasswd_open(SYSPASSWD *,const char *) ;
extern int syspasswd_close(SYSPASSWD *) ;
extern int syspasswd_readent(SYSPASSWD *,struct passwd *,char *,int) ;
extern int syspasswd_reset(SYSPASSWD *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSPASSWD_MASTER */

#endif /* SYSPASSWD_INCLUDE */



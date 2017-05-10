/* kvsfile */


/* revision history:

	= 1998-02-23, David A­D­ Morano
	Originally written for Rightcore Network Services.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	KVSFILE_INCLUDE
#define	KVSFILE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<netdb.h>

#include	<vecobj.h>
#include	<hdb.h>
#include	<localmisc.h>


/* object defines */

#define	KVSFILE			struct kvsfile_head
#define	KVSFILE_CUR		struct kvsfile_c

#define	KVSFILE_MAGIC		0x31415926
#define	KVSFILE_KEYLEN		MAXHOSTNAMELEN
#define	KVSFILE_DEFENTS	100
#define	KVSFILE_DEFFILES	10


struct kvsfile_c {
	HDB_CUR		ec ;
	int		i ;
} ;

struct kvsfile_head {
	uint		magic ;
	time_t		ti_check ;
	vecobj		files ;
	vecobj		keys ;
	HDB		keyvals ;	/* indexed by key-value */
	HDB		entries ;	/* indexed by key */
} ;


typedef struct kvsfile_head	kvsfile ;
typedef struct kvsfile_c	kvsfile_cur ;


#if	(! defined(KVSFILE_MASTER)) || (KVSFILE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int kvsfile_open(KVSFILE *,int,const char *) ;
extern int kvsfile_fileadd(KVSFILE *,const char *) ;
extern int kvsfile_curbegin(KVSFILE *,KVSFILE_CUR *) ;
extern int kvsfile_curend(KVSFILE *,KVSFILE_CUR *) ;
extern int kvsfile_enumkey(KVSFILE *,KVSFILE_CUR *,char *,int) ;
extern int kvsfile_enum(KVSFILE *,KVSFILE_CUR *,char *,int,char *,int) ;
extern int kvsfile_fetch(KVSFILE *,const char *,KVSFILE_CUR *,char *,int) ;
extern int kvsfile_check(KVSFILE *,time_t) ;
extern int kvsfile_close(KVSFILE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* KVSFILE_MASTER */

#endif /* KVSFILE_INCLUDE */



/* svcfile */

/* service file manager */


/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

#ifndef	SVCFILE_INCLUDE
#define	SVCFILE_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<netdb.h>

#include	<hdb.h>
#include	<vecobj.h>
#include	<vechand.h>
#include	<localmisc.h>


/* object defines */

#define	SVCFILE_MAGIC		0x31415926
#define	SVCFILE			struct svcfile_head
#define	SVCFILE_CUR		struct svcfile_c
#define	SVCFILE_ENT		struct svcfile_e

#define	SVCFILE_SVCLEN		MAX(MAXHOSTNAMELEN,1024)
#define	SVCFILE_ENTLEN		2048


struct svcfile_c {
	hdb_cur		ec ;
	int		i ;
} ;

struct svcfile_head {
	uint		magic ;
	time_t		checktime ;
	vecobj		files ;		/* files */
	vecobj		svcnames ;
	hdb		entries ;
	int		ncursors ;
} ;

struct svcfile_e {
	const char	*(*keyvals)[2] ;
	const char	*svc ;
	int		nkeys ;
	int		size ;
	int		fi ;		/* file index */
} ;


typedef struct svcfile_head	svcfile ;
typedef struct svcfile_c	svcfile_cur ;
typedef struct svcfile_e	svcfile_e ;


#if	(! defined(SVCFILE_MASTER)) || (SVCFILE_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int svcfile_open(SVCFILE *,cchar *) ;
extern int svcfile_fileadd(SVCFILE *,cchar *) ;
extern int svcfile_curbegin(SVCFILE *,SVCFILE_CUR *) ;
extern int svcfile_curend(SVCFILE *,SVCFILE_CUR *) ;
extern int svcfile_enumsvc(SVCFILE *,SVCFILE_CUR *,char *,int) ;
extern int svcfile_enum(SVCFILE *,SVCFILE_CUR *,
		SVCFILE_ENT *,char *,int) ;
extern int svcfile_fetch(SVCFILE *,cchar *,SVCFILE_CUR *,
		SVCFILE_ENT *,char *,int) ;
extern int svcfile_match(SVCFILE *,cchar *) ;
extern int svcfile_check(SVCFILE *,time_t) ;
extern int svcfile_close(SVCFILE *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SVCFILE_MASTER */

#endif /* SVCFILE_INCLUDE */



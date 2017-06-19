/* nodedb */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	NODEDB_INCLUDE
#define	NODEDB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<netdb.h>

#include	<vsystem.h>		/* for 'uino_t' */
#include	<vecobj.h>
#include	<hdb.h>
#include	<localmisc.h>


#define	NODEDB_MAGIC		0x31415926
#define	NODEDB			struct nodedb_head
#define	NODEDB_CUR		struct nodedb_c
#define	NODEDB_ENT		struct nodedb_ent

#define	NODEDB_LSVC		NODENAMELEN
#define	NODEDB_LCLU		NODENAMELEN
#define	NODEDB_LSYS		NODENAMELEN

#define	NODEDB_ENTLEN		(3*NODENAMELEN)
#define	NODEDB_DEFENTS		10


struct nodedb_c {
	hdb_cur		ec ;
	int		i ;
} ;

struct nodedb_head {
	uint		magic ;
	vecobj		files ;		/* files */
	hdb		entries ;
	uino_t		pwd_ino ;
	dev_t		pwd_dev ;
	time_t		checktime ;
	int		pwd_len ;
	int		cursors ;
	char		pwd[MAXPATHLEN + 1] ;
} ;

struct nodedb_ent {
	const char	*(*keys)[2] ;
	const char	*svc, *clu, *sys ;
	int		nkeys ;
	int		size ;
	int		fi ;		/* file index */
} ;


#if	(! defined(NODEDB_MASTER)) || (NODEDB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int nodedb_open(NODEDB *,const char *) ;
extern int nodedb_fileadd(NODEDB *,const char *) ;
extern int nodedb_curbegin(NODEDB *,NODEDB_CUR *) ;
extern int nodedb_curend(NODEDB *,NODEDB_CUR *) ;
extern int nodedb_fetch(NODEDB *,cchar *,NODEDB_CUR *,
		NODEDB_ENT *,char *,int) ;
extern int nodedb_enum(NODEDB *,NODEDB_CUR *,NODEDB_ENT *,char *,int) ;
extern int nodedb_check(NODEDB *,time_t) ;
extern int nodedb_close(NODEDB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* NODEDB_MASTER */

#endif /* NODEDB_INCLUDE */



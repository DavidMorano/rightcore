/* srvtab */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

#ifndef	SRVTAB_INCLUDE
#define	SRVTAB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<limits.h>

#include	<vecitem.h>
#include	<localmisc.h>


#define	SRVTAB_MAGIC	0x31415926
#define	SRVTAB		struct srvtab_head
#define	SRVTAB_FILE	struct srvtab_file
#define	SRVTAB_ENT	struct srvtab_e


struct srvtab_head {
	uint		magic ;
	vecitem		e ;		/* server entries */
	time_t		opentime ;	/* time FD was cached */
	time_t		checktime ;	/* time last checked */
	time_t		mtime ;		/* last file modification */
	const char	*fname ;	/* server fname */
	int		fd ;		/* cached server file descriptor */
} ;

struct srvtab_file {
	int		fd ;		/* also serves as "open" flag */
	const char	*fname ;
	time_t		opentime ;	/* time FD was cached */
	time_t		checktime ;	/* time last checked */
	time_t		mtime ;		/* file modification time */
} ;

struct srvtab_e {
	const char	*service ;
	const char	*matchcode ;
	const char	*program ;
	const char	*args ;
	const char	*options ;
	const char	*access ;
	const char	*interval ;
	const char	*addr ;
	const char	*pass ;
	const char	*username ;
	const char	*groupname ;
	const char	*groupnames[NGROUPS_MAX + 1] ;
	const char	*project ;
	int		ngroups ;
	int		matchlen ;
} ;


typedef struct srvtab_head	srvtab ;
typedef struct srvtab_e		srvtab_ent ;


#if	(! defined(SRVTAB_MASTER)) || (SRVTAB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int srvtab_open(SRVTAB *,const char *,VECITEM *) ;
extern int srvtab_match(SRVTAB *,const char *,SRVTAB_ENT **) ;
extern int srvtab_find(SRVTAB *,const char *,SRVTAB_ENT **) ;
extern int srvtab_get(SRVTAB *,int,SRVTAB_ENT **) ;
extern int srvtab_check(SRVTAB *,time_t,VECITEM *) ;
extern int srvtab_close(SRVTAB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SRVTAB_MASTER */

#endif /* SRVTAB_INCLUDE */



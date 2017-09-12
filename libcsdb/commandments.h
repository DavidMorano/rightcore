/* commandments */


/* revision history:

	= 2008-03-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	COMMANDMENTS_INCLUDE
#define	COMMANDMENTS_INCLUDE	1


#include	<envstandards.h>
#include	<sys/types.h>
#include	<localmisc.h>

#include	"cmi.h"


#define	COMMANDMENTS_MAGIC	0x99447248
#define	COMMANDMENTS		struct commandments_head
#define	COMMANDMENTS_INFO	struct commandments_i
#define	COMMANDMENTS_CUR	struct commandments_c
#define	COMMANDMENTS_C		struct commandments_c
#define	COMMANDMENTS_ENT	struct commandments_e
#define	COMMANDMENTS_E		struct commandments_e
#define	COMMANDMENTS_FL		struct commandments_fl
#define	COMMANDMENTS_OBJ	struct commandments_obj
#define	COMMANDMENTS_DBNAME	"ten"


/* this is the shared-object description */
struct commandments_obj {
	const char	*name ;
	uint		objsize ;
	uint		cursize ;
} ;

struct commandments_i {
	time_t		dbtime ;		/* db-time */
	time_t		citime ;		/* idx-time */
	uint		maxent ;
	uint		count ;
} ;

struct commandments_e {
	uint		cn ;			/* commandment number */
} ;

struct commandments_c {
	CMI_CUR		vicur ;
} ;

struct commandments_fl {
	uint		user:1 ;		/* user or system? */
	uint		idx:1 ;
	uint		ids:1 ;
} ;

struct commandments_head {
	uint		magic ;
	COMMANDMENTS_FL	f ;
	CMI		idx ;
	void		*a ;
	const char	*pr ;
	const char	*dbname ;
	const char	*uhome ;		/* user home dir */
	const char 	*fname ;
	const char	*data_db ;
	size_t		size_db ;		/* srouce DB size */
	time_t		ti_db ;			/* source DB m-time */
	time_t		ti_idx ;		/* index modification */
	time_t		ti_map ;		/* map */
	time_t		ti_lastcheck ;		/* last check of file */
	uid_t		uid ;
	uid_t		uid_pr ;
	gid_t		gid_pr ;
	int		ncursors ;
	int		nents ;
	int		maxent ;
} ;


#if	(! defined(COMMANDMENTS_MASTER)) || (COMMANDMENTS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	commandments_open(COMMANDMENTS *,cchar *,cchar *) ;
extern int	commandments_audit(COMMANDMENTS *) ;
extern int	commandments_count(COMMANDMENTS *) ;
extern int	commandments_max(COMMANDMENTS *) ;
extern int	commandments_read(COMMANDMENTS *,char *,int,int) ;
extern int	commandments_get(COMMANDMENTS *,int,char *,int) ;
extern int	commandments_curbegin(COMMANDMENTS *,COMMANDMENTS_CUR *) ;
extern int	commandments_enum(COMMANDMENTS *,COMMANDMENTS_CUR *,
			COMMANDMENTS_ENT *,char *,int) ;
extern int	commandments_curend(COMMANDMENTS *,COMMANDMENTS_CUR *) ;
extern int	commandments_close(COMMANDMENTS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* COMMANDMENTS_MASTER */

#endif /* COMMANDMENTS_INCLUDE */



/* statmsg */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	STATMSG_INCLUDE
#define	STATMSG_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<ptm.h>
#include	<lockrw.h>
#include	<paramfile.h>
#include	<vechand.h>
#include	<localmisc.h>


#define	STATMSG_MAGIC	0x75648942
#define	STATMSG		struct statmsg_head
#define	STATMSG_ID	struct statmsg_id
#define	STATMSG_MAPPER	struct statmsg_mapper
#define	STATMSG_FL	struct statmsg_flags


struct statmsg_id {
	cchar		*groupname ;
	cchar		*username ;
	uid_t		uid ;
	gid_t		gid ;
} ;

struct statmsg_mapper {
	uint		magic ;
	LOCKRW		rwm ;
	PARAMFILE	dirsfile ;
	vechand		mapdirs ;
	cchar		*username ;
	cchar		*userhome ;
	cchar		*fname ;
	time_t		ti_mtime ;
	time_t		ti_check ;
} ;

struct statmsg_flags {
	uint		sorted:1 ;
} ;

struct statmsg_check {
	PTM		m ;
	time_t		ti_lastcheck ;	/* needs mutex protection */
	int		nmaps ;
} ;

struct statmsg_head {
	uint		magic ;
	STATMSG_FL	f ;
	STATMSG_MAPPER	mapper ;
	PTM		m ;		/* this is for all of the data */
	cchar		**envv ;
	cchar		*useralloc ;
	cchar		*username ;
	cchar		*userhome ;
	cchar		*fe ;		/* file-ending */
	time_t		ti_lastcheck ;
	int		nmaps ;
	int		nenv ;
} ;


#if	(! defined(STATMSG_MASTER)) || (STATMSG_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	statmsg_open(STATMSG *,cchar *) ;
extern int	statmsg_check(STATMSG *,time_t) ;
extern int	statmsg_process(STATMSG *,cchar *,cchar **,cchar *,int) ;
extern int	statmsg_processid(STATMSG *,STATMSG_ID *,cchar **,cchar *,int) ;
extern int	statmsg_close(STATMSG *) ;

extern int	statmsgid_load(STATMSG_ID *,cchar *,cchar *,uid_t,gid_t) ;

#ifdef	__cplusplus
}
#endif

#endif /* STATMSG_MASTER */

#endif /* STATMSG_INCLUDE */



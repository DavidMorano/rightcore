/* motd */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	MOTD_INCLUDE
#define	MOTD_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>

#include	<ptm.h>
#include	<lockrw.h>
#include	<paramfile.h>
#include	<finduid.h>
#include	<vechand.h>
#include	<localmisc.h>


#define	MOTD_MAGIC	0x75648941
#define	MOTD		struct motd_head
#define	MOTD_ID		struct motd_id
#define	MOTD_MAPPER	struct motd_mapper
#define	MOTD_FL		struct motd_flags


struct motd_id {
	const char	*groupname ;
	const char	*username ;
	uid_t		uid ;
	gid_t		gid ;
} ;

struct motd_mapper {
	uint		magic ;
	LOCKRW		rwm ;
	PARAMFILE	dirsfile ;
	vechand		mapdirs ;
	const char	*fname ;
	time_t		ti_mtime ;
	time_t		ti_check ;
} ;

struct motd_flags {
	uint		ufind:1 ;
} ;

struct motd_head {
	uint		magic ;
	MOTD_FL		open ;
	MOTD_MAPPER	mapper ;
	PTM		m ;		/* this is for all of the data */
	FINDUID		ufind ;
	const char	**envv ;
	const char	*pr ;
	const char	*fe ;		/* file-ending */
	time_t		ti_lastcheck ;
	int		nmaps ;
	int		nenv ;
} ;


#if	(! defined(MOTD_MASTER)) || (MOTD_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	motd_open(MOTD *,const char *) ;
extern int	motd_check(MOTD *,time_t) ;
extern int	motd_process(MOTD *,const char *,const char **,int) ;
extern int	motd_processid(MOTD *,MOTD_ID *,const char **,int) ;
extern int	motd_close(MOTD *) ;

extern int	motdid_load(MOTD_ID *,const char *,const char *,uid_t,gid_t) ;

#ifdef	__cplusplus
}
#endif

#endif /* MOTD_MASTER */

#endif /* MOTD_INCLUDE */



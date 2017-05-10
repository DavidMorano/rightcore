/* systems */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	SYSTEMS_INCLUDE
#define	SYSTEMS_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>

#include	<vecobj.h>


#define	SYSTEMS_CUR	struct systems_c
#define	SYSTEMS_ENT	struct systems_ent
#define	SYSTEMS		struct systems_head
#define	SYSTEMS_MAGIC	31415926


struct systems_c {
	int		i ;
} ;

struct systems_ent {
	const char	*sysname ;
	const char	*dialername ;
	const char	*dialerargs ;
	int		fi ;
	int		sysnamelen ;
	int		dialernamelen ;
	int		dialerargslen ;
} ;

struct systems_head {
	uint		magic ;
	vecobj		files ;
	vecobj		entries ;
	time_t		checktime ;
} ;


#if	(! defined(SYSTEMS_MASTER)) || (SYSTEMS_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	systems_open(SYSTEMS *,const char *) ;
extern int	systems_fileadd(SYSTEMS *,const char *) ;
extern int	systems_filedel(SYSTEMS *,const char *) ;
extern int	systems_check(SYSTEMS *,time_t) ;
extern int	systems_curbegin(SYSTEMS *,SYSTEMS_CUR *) ;
extern int	systems_curend(SYSTEMS *,SYSTEMS_CUR *) ;
extern int	systems_enum(SYSTEMS *,SYSTEMS_CUR *,SYSTEMS_ENT **) ;
extern int	systems_fetch(SYSTEMS *,cchar *,SYSTEMS_CUR *,SYSTEMS_ENT **) ;
extern int	systems_close(SYSTEMS *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SYSTEMS_MASTER */

#endif /* SYSTEMS_INCLUDE */



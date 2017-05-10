/* dialtab */


/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

#ifndef	DIALTAB_INCLUDE
#define	DIALTAB_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>

#include	<vecobj.h>


#define	DIALTAB_ENT	struct dialtab_ent
#define	DIALTAB		struct dialtab_head


struct dialtab_ent {
	const char	*name ;
	const char	*inet ;
	const char	*uucp ;
	const char	*username ;
	const char	*password ;
	int		fi ;
} ;

struct dialtab_head {
	uint		magic ;
	vecobj		files ;
	vecobj		entries ;
} ;


#if	(! defined(DIALTAB_MASTER)) || (DIALTAB_MASTER == 0)

#ifdef	__cplusplus
extern "C" {
#endif

extern int	dialtab_open(DIALTAB *,const char *) ;
extern int	dialtab_fileadd(DIALTAB *,const char *) ;
extern int	dialtab_check(DIALTAB *,const char *) ;
extern int	dialtab_search(DIALTAB *,const char *,DIALTAB_ENT **) ;
extern int	dialtab_close(DIALTAB *) ;

#ifdef	__cplusplus
}
#endif

#endif /* DIALTAB_MASTER */

#endif /* DIALTAB_INCLUDE */



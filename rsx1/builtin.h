/* builtin */


/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */


#ifndef	BUILTIN_INCLUDE
#define	BUILTIN_INCLUDE		1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<time.h>

#include	<vecstr.h>
#include	<svcfile.h>
#include	<connection.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"standing.h"


#define	BUILTIN			struct builtin


struct builtin_cache {
	time_t			boottime ;
} ;

struct builtin {
	unsigned long		magic ;
	struct builtin_cache	c ;
	SVCFILE			*sfp ;
	struct proginfo		*pip ;
	uint		providerid ;
	int		nentries ;
	int		hostnamelen ;
} ;


#if	(! defined(BUILTIN_MASTER)) || (BUILTIN_MASTER == 0)

extern int builtin_start(BUILTIN *,struct proginfo *) ;
extern int builtin_finish(BUILTIN *) ;
extern int builtin_match(BUILTIN *,const char *) ;
extern int builtin_enum(BUILTIN *,int,const char **) ;
extern int builtin_execute(BUILTIN *,STANDING *,struct clientinfo *,int,
				const char **) ;

#endif /* BUILTIN_MASTER */


#endif /* BUILTIN_INCLUDE */




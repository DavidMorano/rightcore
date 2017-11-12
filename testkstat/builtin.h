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
#include	"clientinfo.h"
#include	"standing.h"


#define	BUILTIN_MAGIC	0x12345678
#define	BUILTIN		struct builtin
#define	BUILTIN_CACHE	struct builtin_cache


struct builtin_cache {
	time_t		boottime ;
} ;

struct builtin {
	unsigned long	magic ;
	BUILTIN_CACHE	c ;
	SVCFILE		*sfp ;
	PROGINFO	*pip ;
	uint		providerid ;
	int		nentries ;
	int		hostnamelen ;
} ;


#if	(! defined(BUILTIN_MASTER)) || (BUILTIN_MASTER == 0)

extern int builtin_start(BUILTIN *,PROGINFO *) ;
extern int builtin_finish(BUILTIN *) ;
extern int builtin_match(BUILTIN *,const char *) ;
extern int builtin_enum(BUILTIN *,int,const char **) ;
extern int builtin_execute(BUILTIN *,STANDING *,CLIENTINFO *,int,cchar **) ;

#endif /* BUILTIN_MASTER */


#endif /* BUILTIN_INCLUDE */



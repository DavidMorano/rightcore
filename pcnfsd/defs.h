/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>


struct flags {
	uint	debug : 1 ;
	uint	logging : 1 ;
	uint	sysv_ct : 1 ;
	uint	shadow : 1 ;
	uint	useshadow : 1 ;
} ;

struct global {
	struct flags	f ;
	bfile		*efp ;
	bfile		*lfp ;
	int		debuglevel ;
	time_t		daytime ;
	char		*nodename ;
	char		*domain ;
	char		*progname ;
	char		*logfname ;
	char		*spooldir ;
} ;


#define	MAXPARGS	10


#ifndef	TYPEDEF_UTIMET
#define	TYPEDEF_UTIMET	1
typedef	unsigned long	utime_t ;
#endif


#endif /* DEFS_INCLUDE */



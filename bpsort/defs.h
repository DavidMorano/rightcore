/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>

#include	<bfile.h>
#include	<localmisc.h>


#define	BUFLEN		100


struct proginfo_flags {
	uint	outfile : 1 ;
	uint	errfile : 1 ;
	uint	quiet : 1 ;
} ;

struct proginfo {
	bfile		*efp ;
	char		*version ;
	char		*progname ;
	char		*programroot ;
	char		*helpfile ;
	struct proginfo_flags	f ;
	int		debuglevel ;
	int		verboselevel ;
} ;

struct entry {
	double		accuracy ;
	char		*progname ;
	char		*bpname ;
	char		*fname ;
	uint		bits ;
	uint		p1, p2, p3 , p4 ;
	int		line ;
} ;


#endif /* DEFS_INCLUDE */



/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<stdio.h>

#include	<localmisc.h>


#ifndef	FD_STDIN
#define	FD_STDIN	0
#define	FD_STDOUT	1
#define	FD_STDERR	2
#endif



struct proginfo_flags {
	uint	outfile : 1 ;
	uint	errfile : 1 ;
	uint	literal : 1 ;
	uint	trunclen : 1 ;
	uint	quiet : 1 ;
} ;

struct proginfo {
	FILE		*efp ;
	char		*version ;
	char		*progname ;
	char		*programroot ;
	char		*helpfname ;
	struct proginfo_flags	f ;
	int		debuglevel ;
	int		verboselevel ;
} ;


#endif /* DEFS_INCLUDE */



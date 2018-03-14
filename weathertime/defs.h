/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/timeb.h>

#include	<date.h>
#include	<localmisc.h>


#define	BUFLEN		100

#define	NISDOMAINNAME	"/etc/defaultdomain"

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint	no : 1 ;		/* NOOP */
	uint	quiet : 1 ;
	uint	outopen : 1 ;		/* is STDOUT open ? */
} ;

struct proginfo {
	PROGINFO_FL	f ;
	bfile		*efp ;
	bfile		*ofp ;
	char		*progname ;
	char		*version ;
	char		*searchname ;
	char		*programroot ;
	char		*helpfile ;
	struct timeb	now ;
	DATE		tmpdate ;
	int		debuglevel ;
	int		verboselevel ;
	char		zname[DATE_ZNAMESIZE + 1] ;
} ;


#endif /* DEFS_INCLUDE */



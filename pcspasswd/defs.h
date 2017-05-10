/* defs */

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */


#ifndef	DEFS_INCLUDE
#define	DEFS_INCLUDE	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<vecstr.h>
#include	<localmisc.h>


#ifndef	BUFLEN
#define	BUFLEN		MAXPATHLEN
#endif

#ifndef	PWENTRY_RECLEN
#define	PWENTRY_RECLEN	256
#endif

#define	PASSLEN		32
#define	PROMPTLEN	100

#define	PROGINFO	struct proginfo
#define	PROGINFO_FL	struct proginfo_flags

#define	PIVARS		struct pivars

#define	ARGINFO		struct arginfo


struct proginfo_flags {
	uint	aparams : 1 ;
	uint	nooutput : 1 ;
	uint	quiet : 1 ;
	uint	sevenbit : 1 ;
	uint	update : 1 ;
} ;

struct proginfo {
	vecstr		stores ;
	char		**envv ;
	char		*pwd ;
	char		*progename ;
	char		*progdname ;
	char		*progname ;
	char		*pr ;
	char		*searchname ;
	char		*version ;
	char		*banner ;
	char		*tmpdname ;
	void		*efp ;
	void		*ofp ;
	struct proginfo_flags	f ;
	struct proginfo_flags	open ;
	int		pwdlen ;
	int		debuglevel ;
	int		verboselevel ;
} ;


#endif /* DEFS_INCLUDE */



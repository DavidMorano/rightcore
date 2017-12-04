/* svckey */


/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

#ifndef	SVCKEY_INCLUDE
#define	SVCKEY_INCLUDE	1


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>

#include	<svcfile.h>


#define	SVCKEY		struct svckey


enum svckeys {
	svckey_file,
	svckey_so,
	svckey_p,
	svckey_pass,
	svckey_a,
	svckey_u,
	svckey_g,
	svckey_interval,
	svckey_acc,
	svckey_opts,
	svckey_failcont,
	svckey_include,
	svckey_overlast
} ;

struct svckey {
	const char	*file ;
	const char	*pass ;		/* pass-file (pipe) */
	const char	*svc ;		/* service */
	const char	*so ;		/* shared object */
	const char	*p ;		/* program (execfname) */
	const char	*a ;		/* arguments */
	const char	*u ;		/* username */
	const char	*g ;		/* group */
	const char	*acc ;		/* access */
	const char	*interval ;	/* interval */
	const char	*opts ;		/* options */
	const char	*failcont ;	/* fail-continue */
	const char	*include ;
} ;


#ifdef	__cplusplus
extern "C" {
#endif

extern int	svckey_load(struct svckey *,SVCFILE_ENT *) ;

#ifdef	__cplusplus
}
#endif

#endif /* SVCKEY_INCLUDE */



/* isfnamespecial */

/* test if a given file-name is special */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 1998-02-24, David A­D­ Morano
        This code was adopted from the SHCAT program, which in turn had been
        adopted from programs with a lineage dating back (from the previous
        notes in this space) from 1989! I deleted the long list of notes here to
        clean this up.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We test if a given file-name is special.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<localmisc.h>


/* local defines */

#ifndef	STDINFNAME
#define	STDINFNAME	"*STDIN*"
#endif

#ifndef	STDOUTFNAME
#define	STDOUTFNAME	"*STDOUT*"
#endif

#ifndef	STDERRFNAME
#define	STDERRFNAME	"*STDERR*"
#endif

#ifndef	STDNULLFNAME
#define	STDNULLFNAME	"*STDNULL*"
#endif


/* external subroutines */

extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matpstr(const char **,int,const char *,int) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */


/* local variables */

static const char	*fnames[] = {
	STDINFNAME,
	STDOUTFNAME,
	STDERRFNAME,
	STDNULLFNAME,
	NULL
} ;


/* exported subroutines */


int isfnamespecial(const char *fp,int fl)
{
	return (matstr(fnames,fp,fl) >= 0) ;
}
/* end subroutine (isfnamespecial) */



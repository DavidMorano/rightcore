/* processargs */

/* process server file program arguments */
/* version %I% last modified %G% */


#define	CF_DEBUG	1		/* compile-time debugging */


/* revision history:

	= 1991-09-01, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 1991 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	These subroutines are used to parse the SERVER program arguments
	from an expanded (substituted) server file entry.  Basically,
	we just "field-SHELL" out arguments and put them into the
	supplied vector string object.


*****************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<pwd.h>
#include	<grp.h>
#include	<netdb.h>

#include	<vsystem.h>
#include	<baops.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN	(4 * MAXPATHLEN)
#endif


/* external subroutines */

extern char	*strbasename() ;


/* externals variables */


/* forward references */


/* local global variables */


/* local structures */


/* exported subroutines */


int processargs(pip,args,alp)
struct proginfo	*pip ;
char		args[] ;
vecstr		*alp ;
{
	FIELD	fsb ;

	int	rs = SR_OK ;
	int	fl ;
	int	i = 0 ;

	uchar	terms[32] ;
	char	fbuf[BUFLEN + 1] ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("processargs: entered> %s\n",args) ;
#endif

	if ((args == NULL) || (args[0] == '\0'))
	    return 0 ;

	fieldterms(terms,0," \t") ;

	if ((rs = field_start(&fsb,args,-1)) >= 0) {

	while ((fl = field_sharg(&fsb,terms,fbuf,BUFLEN)) > 0) {

	    i += 1 ;
	    rs = vecstr_add(alp,fbuf,fl) ;

	    if (rs < 0) break ;
	} /* end while */

	    field_finish(&fsb) ;
	} /* end if (field) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("processargs: exiting, %d args\n",i) ;
#endif

	return (rs >= 0) ? i : rs ;
}
/* end subroutine (processargs) */



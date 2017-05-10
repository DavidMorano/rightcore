/* progsvcargs */

/* process server file program arguments */
/* version %I% last modified %G% */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

        This subroutine is used to parse the SERVER program arguments from an
        expanded (substituted) server file entry. Basically, we just
        "field-SHELL" out arguments and put them into the supplied vector string
        object.


*****************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<netdb.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<baops.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	FBUFLEN
#define	FBUFLEN		(4 * MAXPATHLEN)
#endif


/* external subroutines */


/* externals variables */


/* forward references */


/* local structures */


/* exported subroutines */


int progsvcargs(pip,alp,args)
struct proginfo	*pip ;
vecstr		*alp ;
const char	args[] ;
{
	FIELD	fsb ;

	int	rs = SR_OK ;
	int	fl ;
	int	i = 0 ;

	uchar	terms[32] ;
	char	fbuf[FBUFLEN + 1] ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("progsvcargs: entered> %s\n",args) ;
#endif

	if ((args == NULL) || (args[0] == '\0'))
	    return 0 ;

	fieldterms(terms,0," \t") ;

	rs = field_start(&fsb,args,-1) ;
	if (rs < 0)
	    goto ret0 ;

/* loop through the arguments */

	i = 0 ;
	while ((fl = field_sharg(&fsb,terms,fbuf,FBUFLEN)) >= 0) {

	    i += 1 ;
	    rs = vecstr_add(alp,fbuf,fl) ;

	    if (rs < 0) break ;
	} /* end while */

	field_finish(&fsb) ;

ret0:
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (progsvcargs) */




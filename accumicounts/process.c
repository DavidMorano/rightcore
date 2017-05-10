/* process */

/* process a name */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 2004-03-01, David A­D­ Morano

	The was newly written for the present purpose.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine reads, parses, and adds up 'icounts'
	file a specified file.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<termios.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<wdt.h>
#include	<localmisc.h>

#include	"paramopt.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	nextfield(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;


/* external variables */


/* local structures */


/* forward references */


/* global variables */


/* local variables */







int process(pip,name,pp)
struct proginfo	*pip ;
char		name[] ;
PARAMOPT	*pp ;
{
	struct ustat	sb ;

	bfile	ifile ;

	int	rs ;
	int	len ;
	int	sl, cl ;

	const char	*sp, *cp ;

	char	linebuf[LINEBUFLEN + 1] ;


	if (name == NULL)
	    return BAD ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("process: entered name=\"%s\"\n",name) ;
#endif

	if (u_stat(name,&sb) < 0)
	    return BAD ;

	if ((rs = bopen(&ifile,name,"r",0666)) >= 0) {

	    int	ii, n ;


	    while ((rs = breadline(&ifile,linebuf,LINEBUFLEN)) > 0) {

	        len = rs ;
	        if (linebuf[len - 1] == '\n')
	            len -= 1 ;

	        linebuf[len] = '\0' ;

	        sp = linebuf ;
	        sl = len ;
	        cl = nextfield(sp,sl,&cp) ;

	        if (cl < 1)
	            continue ;

	        rs = cfdeci(cp,cl,&ii) ;

	        if ((rs < 0) || (ii < 0))
	            break ;

	        sl -= ((cp + cl) - sp) ;
	        sp = (cp + cl) ;

	        cl = nextfield(sp,sl,&cp) ;

	        if (cl < 1)
	            continue ;

	        rs = cfdeci(cp,cl,&n) ;

	        if ((rs < 0) || (n < 0))
	            break ;

	        if (ii >= ICOUNTS_TOTAL)
	            ii = ICOUNTS_TOTAL ;

	        pip->icounts[ii] += n ;

	    } /* end while */

	    bclose(&ifile) ;

	} /* end if */

	return rs ;
}
/* end subroutine (process) */




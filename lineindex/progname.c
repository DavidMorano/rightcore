/* progname */

/* process a name */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The program was written from scratch to do what the previous
	program by the same name did.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine processes one name at a time.  The name is a
	composit of the name of the file to be indexed and the name of
	the file to create (the index itself).

	Names are in the form:

		file=indexfile


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<baops.h>
#include	<field.h>
#include	<wdt.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"lineindex.h"



/* local defines */

#ifndef	DEBUGLEVEL
#define	DEBUGLEVEL(n)	(pip->debuglevel >= (n))
#endif



/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int progname(pip,name)
struct proginfo	*pip ;
const char	name[] ;
{
	LINEINDEX	li ;

	int	rs ;
	int	line ;
	int	sl, cl ;
	int	oflags, operm ;

	char	fname[MAXPATHLEN + 1] ;
	char	ifname[MAXPATHLEN + 1] ;
	char	*sp, *cp ;


	if (name == NULL)
	    return SR_FAULT ;

/* parse out the two file names */

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progname: name=%s\n",name) ;
#endif

	if ((cp = strchr(name,'=')) == NULL)
		return SR_INVALID ;

	strwcpy(fname,name,MIN((cp - name),MAXPATHLEN)) ;

	strwcpy(ifname,(cp + 1),MAXPATHLEN) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progname: fname=%s ifname=%s\n",fname,ifname) ;
#endif

	oflags = (O_RDWR | O_CREAT) ;
	operm = 0666 ;
	rs = lineindex_open(&li,ifname,oflags,operm,fname) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progname: lineindex_open() rs=%d\n",rs) ;
#endif

	if (rs >= 0) {

#if	CF_DEBUG
	if (DEBUGLEVEL(5)) {
		LINEINDEX_CUR	cur ;
		offset_t		off ;
			debugprintf("progname: fname=%s\n",fname) ;
		lineindex_curbegin(&li,&cur) ;
		line = 0 ;
		while (lineindex_enum(&li,&cur,&off) >= 0) {
			debugprintf("progname: line=%u off=%lu\n",
				line,off) ;
			line += 1 ;
		} /* end while */
		lineindex_curend(&li,&cur) ;
	}
#endif /* CF_DEBUG */

		rs = lineindex_close(&li) ;

	} /* end if (opened the index) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("progname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progname) */




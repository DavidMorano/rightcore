/* process */

/* process an input file */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1996-03-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine processes one name at a time. The name can be either a
        file or a directory.


*******************************************************************************/


#include	<envstandards.h>

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
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* global variables */


/* local structures */

struct record {
	char		fname[100] ;
	char		lname[100] ;
	char		phone[20] ;
	char		street[100] ;
	char		rest[100] ;
	char		municpality[100] ;
	char		principality[100] ;
	char		zip[20] ;
} ;


/* forward references */

static int	process_record(PROGINFO *,bfile *,struct record *) ;


/* local variables */


/* exported subroutines */


int process(PROGINFO *pip,cchar *fname)
{
	struct ustat	sb, sb2 ;
	struct record	e ;
	bfile		infile, *ifp = &infile ;
	int		rs, rl ;

	if (fname == NULL) return SR_FAULT ;

	if ((fname[0] == '\0') || (fname[0] == '-')) 
	    fname = BFILE_STDIN ;

	if ((rs = bopen(ifp,fname,"rd",0666)) >= 0) {

	while ((rs = process_record(pip,ifp,&e)) > 0) {

		bprintf(pip->ofp,
			"%s|%s|%s|%s|%s\n",
			e.fname,
			e.lname,
			e.street,
			e.rest,
			e.phone) ;

	} /* end while */

	bclose(ifp) ;
	} /* end if (file-process) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("process: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (process) */


/* local subroutine */


static int process_record(pip,ifp,ep)
PROGINFO	*pip ;
bfile		*ifp ;
struct record	*ep ;
{
	const int	llen = LINEBUFLEN ;
	int		rs ;
	int		ll, sl, cl ;
	const char	*sp, *cp ;
	char		lbuf[LINEBUFLEN + 1] ;

	while ((ll = breadline(ifp,lbuf,llen)) > 0) {
		sl = sfshrink(lbuf,ll,&sp) ;
		if (sl > 0) break ;
	} /* end while */

	if (ll <= 0)
		return ll ;

	strwcpy(ep->fname,sp,MIN(100,sl)) ;

/* last name */

	ll = breadline(ifp,lbuf,LINEBUFLEN) ;

	if (ll <= 0)
		return ll ;

	sl = sfshrink(lbuf,ll,&sp) ;

	strwcpy(ep->lname,sp,MIN(100,sl)) ;

/* phone */

	ll = breadline(ifp,lbuf,LINEBUFLEN) ;

	if (ll <= 0)
		return ll ;

	sl = sfshrink(lbuf,ll,&sp) ;

	strwcpy(ep->phone,sp,MIN(20,sl)) ;

/* street */

	ll = breadline(ifp,lbuf,LINEBUFLEN) ;

	if (ll <= 0)
		return ll ;

	sl = sfshrink(lbuf,ll,&sp) ;

	strwcpy(ep->street,sp,MIN(100,sl)) ;

/* rest */

	ll = breadline(ifp,lbuf,LINEBUFLEN) ;

	if (ll <= 0)
		return ll ;

	sl = sfshrink(lbuf,ll,&sp) ;

	strwcpy(ep->rest,sp,MIN(100,sl)) ;

	return 1 ;
}
/* end subroutine (process_record) */



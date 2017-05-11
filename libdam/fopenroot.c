/* frcopenroot */

/* open a file name according to rules */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2001-04-11, David A­D­ Morano

	This was straightforwardly adapted from 'bopenroot()'.


*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*****************************************************************************

	This subroutine will form a file name according to some
	rules.

	The rules are roughly:

	+ attempt to open it directly if it is already rooted
	+ open it if it is already in the root area
	+ attempt to open it as it is if it already exists
	+ attempt to open or create it located in the root area
	+ attempt to open or create it as it is

	Synopsis:

	int frcopenroot(fpp,programroot,fname,outname,mode)
	FILE	**fpp ;
	char	programroot[], fname[], outname[] ;
	char	mode[] ;

	Arguments:

	+ fp		pointer to 'bfile' object
	+ programroot	path of program root directory
	+ fname		fname to find and open
	+ outname	user supplied buffer to hold possible resulting name
	+ mode		file open mode

	Returns:

	>=0		success (same as 'bopen()')
	<0		error (same as 'bopen()')

	outname		1. zero length string if no new name was needed
			2. will contain the path of the file that was opened

	
*****************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"outbuf.h"


/* local defines */


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;

extern char	*strwcpy(char *,char *,int) ;


/* external variables */


/* forward references */

static int	frcopen(FILE **,char *,char *) ;


/* local structures */


/* local variables */


/* exported subroutines */


int frcopenroot(fpp,programroot,fname,outname,mode)
FILE	**fpp ;
char	programroot[], fname[], outname[] ;
char	mode[] ;
{
	OUTBUF	ob ;

	FILE	*fp ;

	int	rs ;
	int	sl ;
	int	imode ;
	int	f_outbuf ;

	char	*mp, *onp = NULL ;


#if	CF_DEBUGS
	debugprintf("bopenroot: ent fname=%s\n",fname) ;
#endif

	if ((fname == NULL) || (fname[0] == '\0'))
	    return SR_NOEXIST ;

	if (fpp == NULL)
	    return SR_FAULT ;

	f_outbuf = (outname != NULL) ;

	imode = 0 ;
	for (mp = mode ; *mp ; mp += 1) {

	    switch ((int) *mp) {

	    case 'r':
	        imode += R_OK ;
	        break ;

	    case '+':
	    case 'w':
	        imode += W_OK ;
	        break ;

	    case 'x':
	        imode += X_OK ;
	        break ;

	    } /* end switch */

	} /* end for */

	if (fname[0] == '/') {

	    if (f_outbuf)
	        outname[0] = '\0' ;

	    rs = frcopen(fpp,fname,mode) ;

	    goto ret0 ;
	}

	rs = outbuf_start(&ob,outname,-1) ;
	if (rs < 0)
	    goto ret0 ;

	if (programroot != NULL) {

#if	CF_DEBUGS
	    debugprintf("bopenroot: about to alloc\n") ;
#endif

	    rs = outbuf_get(&ob,&onp) ;
	    if (rs < 0)
	        goto ret1 ;

	    sl = mkpath2(onp,programroot,fname) ;

	    if (perm(onp,-1,-1,NULL,imode) >= 0) {

	        rs = frcopen(fpp,onp,mode) ;
	        if (rs >= 0)
	            goto done ;

	    }

	}

	if (perm(fname,-1,-1,NULL,imode) >= 0) {

	    rs = frcopen(fpp,fname,mode) ;
	    if (rs >= 0) {

	        if (f_outbuf)
	            outname[0] = '\0' ;

	        goto done ;
	    }
	}

	if ((programroot != NULL) &&
	    (strchr(fname,'/') != NULL)) {

	    rs = frcopen(fpp,onp,mode) ;
	    if (rs >= 0)
	        goto done ;

	}

	if (f_outbuf)
	    outname[0] = '\0' ;

	rs = frcopen(fpp,fname,mode) ;

done:
ret1:
	outbuf_finish(&ob) ;

ret0:
	return rs ;
}
/* end subroutine (frcopenroot) */


/* local subroutines */


/* file open w/ return code */
static int frcopen(fpp,fname,mode)
FILE	**fpp ;
char	fname[] ;
char	mode[] ;
{


	*fpp = fopen(fname,mode) ;

	return ((*fpp != NULL) ? SR_OK : SR_NOTOPEN) ;
}
/* end subroutine (frcopen) */




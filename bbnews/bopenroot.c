/* bopenroot */

/* open a file name according to rules */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will form a file name according to some rules.

	The rules are roughly:

	+ attempt to open it directly if it is already rooted
	+ open it if it is already in the root area
	+ attempt to open it as it is if it already exists
	+ attempt to open or create it located in the root area
	+ attempt to open or create it as it is

	Synopsis:

	int bopenroot(fp,pr,fname,outfname,mode,operms)
	bfile		*fp ;
	const char	pr[], fname[] ;
	const char	mode[] ;
	char		outfname[] ;
	mode_t		operms ;

	Arguments:

	fp		pointer to 'bfile' object
	pr		path of program root directory
	fname		fname to find and open
	mode		file open mode
	outfname	user supplied buffer to hold possible resulting name
	operms		file opermss to use in the open 

	Returns:

	>=0		success (same as 'bopen()')
	<0		error (same as 'bopen()')

	outfname	1. zero length string if no new name was needed
			2. will contain the path of the file that was opened


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<outbuf.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;


/* external variables */


/* forward references */


/* local structures */


/* local variables */


/* exported subroutines */


int bopenroot(fp,pr,fname,outfname,mode,operms)
bfile		*fp ;
const char	pr[] ;
const char	fname[] ;
const char	mode[] ;
char		outfname[] ;
mode_t		operms ;
{
	OUTBUF		ob ;
	int		rs = SR_OK ;
	int		imode ;
	int		f_outbuf = FALSE ;
	const char	*mp ;
	char		*onp = NULL ;

	if (fp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;
	if (mode == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;
	if (mode[0] == '\0') return SR_INVALID ;

	f_outbuf = (outfname != NULL) ;

	imode = 0 ;
	for (mp = mode ; *mp ; mp += 1) {

	    switch ((int) *mp) {

	    case 'r':
	        imode += R_OK ;
	        break ;

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
	        outfname[0] = '\0' ;

	    rs = bopen(fp,fname,mode,operms) ;

	    goto ret0 ;

	} /* end if */

	rs = outbuf_start(&ob,outfname,-1) ;
	if (rs < 0)
	    goto ret0 ;

	if (pr != NULL) {

	    rs = outbuf_get(&ob,&onp) ;
	    if (rs < 0)
	        goto done ;

	    rs = mkpath2(onp, pr,fname) ;

	    if (rs >= 0)
	        rs = perm(onp,-1,-1,NULL,imode) ;

	    if (rs >= 0)
	        rs = bopen(fp,onp,mode,operms) ;

	    if (rs >= 0)
	        goto done ;

	} /* end if (we had a pr) */

	if ((perm(fname,-1,-1,NULL,imode) >= 0) &&
	    ((rs = bopen(fp,fname,mode,operms)) >= 0)) {

	    if (f_outbuf)
	        outfname[0] = '\0' ;

	    goto done ;
	}

	if ((pr != NULL) &&
	    (strchr(fname,'/') != NULL)) {

	    rs = bopen(fp,onp,mode,operms) ;

	    if (rs >= 0)
	        goto done ;

	}

	if (f_outbuf)
	    outfname[0] = '\0' ;

	rs = bopen(fp,fname,mode,operms) ;

done:
	outbuf_finish(&ob) ;

ret0:
	return rs ;
}
/* end subroutine (bopenroot) */



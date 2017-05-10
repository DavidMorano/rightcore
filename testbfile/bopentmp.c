/* bopentmp */

/* open a tempory file */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_MKDIRS	1		/* make any necessary directories */


/* revision history:

	= 2004-04-13, David A­D­ Morano

	This program was originally written.


*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will open up a temporary file.  If successful,
	the file is deleted before this subroutine returns.

	Synopsis:

	int bopentmp(fp,tname,ostr,operms)
	bfile		*fp ;
	const char	tname[] ;
	const char	ostr[] ;
	mode_t		operms ;

	Arguments:

	fp		object pointer
	tname		template file name
	ostr		file-open mode
	operms		file permissions

	Returns:

	>=0		OK
	<0		error


*******************************************************************************/

#define	BFILE_MASTER	0

#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<sigblock.h>
#include	<localmisc.h>

#include	"bfile.h"


/* local defines */

#ifndef	VARTMPDNAME
#define	VARTMPDNAME	"TMPDIR"
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif

#ifndef	BFILE_TEMPLATE
#define	BFILE_TEMPLATE	"btmpXXXXXXXXXX"
#endif


/* external subroutines */

extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkdirs(const char *,mode_t) ;
extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	sfdirname(const char *,int,const char **) ;


/* external variables */


/* local structures */


/* forward references */

static int mktmpdirs(const char *,mode_t) ;


/* local variables */


/* exported subroutines */


int bopentmp(fp,tname,ostr,operms)
bfile		*fp ;
const char	tname[] ;
const char	ostr[] ;
mode_t		operms ;
{
	int	rs = SR_OK ;

	char	xfname[MAXPATHLEN + 1] ;


	if (fp == NULL) return SR_FAULT ;
	if (ostr == NULL) return SR_FAULT ;

	if (ostr[0] == '\0') return SR_INVALID ;

	if ((tname == NULL) || (tname[0] == '\0')) tname = BFILE_TEMPLATE ;

	if (tname[0] != '/') {
	    const char	*tmpdname = getenv(VARTMPDNAME) ;
	    if (tmpdname == NULL) tmpdname = TMPDNAME ;
	    rs = mkpath2(xfname,tmpdname,tname) ;
	} else
	    rs = mkpath1(xfname,tname) ;

#if	CF_MKDIRS
	if (rs >= 0)
	    rs = mktmpdirs(xfname,operms) ;
#endif /* CF_MKDIRS */

	if (rs >= 0) {
	    SIGBLOCK	b ;
	    if ((rs = sigblock_start(&b,NULL)) >= 0) {
		char	tmpfname[MAXPATHLEN + 1] ;
	
	        if ((rs = mktmpfile(tmpfname,operms,xfname)) >= 0) {
	            rs = bopen(fp,tmpfname,ostr,operms) ;
	            u_unlink(tmpfname) ;
	        } /* end if (tmp-file) */

	        sigblock_finish(&b) ;
	    } /* end if (sigblock) */
	} /* end if */

	return rs ;
}
/* end subroutine (bopentmp) */


/* local subroutines */


#if	CF_MKDIRS
static int mktmpdirs(const char *xfname,mode_t operms)
{
	int	rs = SR_OK ;
	int	cl ;

	const char	*cp ;

	if ((cl = sfdirname(xfname,-1,&cp)) > 0) {
	    char	dname[MAXPATHLEN+1] ;
	    if ((rs = mkpath1w(dname,cp,cl)) >= 0)
		rs = mkdirs(dname,operms) ;
	}

	return rs ;
} 
/* end subrouine (mktmpdirs) */
#endif /* CF_MKDIRS */




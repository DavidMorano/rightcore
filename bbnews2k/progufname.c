/* progufname */

/* program user-newsrc file */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1995-05-01, David A­D­ Morano

        This code module was completely rewritten to replace any original
        garbage that was here before.


*/

/* Copyright © 1995,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	We figure ot the user-newsrc file here.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	snsds(char *,int,const char *,const char *) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	matocasestr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*timestr_log(time_t,char *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int progufname(PROGINFO *pip,cchar ufname[])
{
	int		rs = SR_OK ;
	int		unl = -1 ;
	cchar		*unp = ufname ;
	char		tbuf[MAXPATHLEN+1] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progufname: ent ufn=%s\n",ufname) ;
#endif

	if (ufname == NULL) {
	    cchar	*def = DEFNEWSRC ;
	    rs = mkpath2(tbuf,pip->homedname,def) ;
	    unl = rs ;
	    unp = tbuf ;
	}

	if (rs >= 0) {
	   cchar	**vpp = &pip->ufname ;
	   rs = proginfo_setentry(pip,vpp,unp,unl) ;
	}

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	debugprintf("progufname: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (progufname) */



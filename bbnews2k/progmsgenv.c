/* progmsgenv */

/* create an environment data */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1995-05-01, David A­D­ Morano
        This code module was completely rewritten to replace any original
        garbage that was here before.

	= 1998-11-22, David A­D­ Morano
        I did some clean-up.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

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

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* global variables */


/* local structures */


/* forward references */

static int	progmsgenv_beginer(PROGINFO *pip) ;


/* local variables */


/* exported subroutines */


int progmsgenv_begin(PROGINFO *pip)
{
	int	rs = SR_OK ;

	if (pip == NULL) return SR_FAULT ;

	return rs ;
}
/* end subroutine (progmsgenv_begin) */


int progmsgenv_end(PROGINFO *pip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (pip->open.envdate) {
	    pip->open.envdate = FALSE ;
	    rs1 = dater_finish(&pip->envdate) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (progmsgenv_end) */


int progmsgenv_envstr(PROGINFO *pip,char mbuf[],int mlen)
{
	int		rs ;

	if ((rs = progmsgenv_beginer(pip)) >= 0) {
	    rs = dater_mkstd(&pip->envdate,mbuf,mlen) ;
	}

	return rs ;
}
/* end subroutine (progmsgenv_envstr) */


/* local subroutines */


static int progmsgenv_beginer(PROGINFO *pip)
{
	int	rs = SR_OK ;

#if	CF_DEBUG
	if (DEBUGLEVEL(5))
	debugprintf("progmsgenv_beginer: zn=%s\n",pip->zname) ;
#endif

	if (! pip->open.envdate) {
	    struct timeb	*nowp = &pip->now ;
	    DATER		*dp = &pip->envdate ;
	    const char		*zn = pip->zname ;
	    if ((rs = dater_start(dp,nowp,zn,-1)) >= 0) {
		time_t		t = pip->daytime ;
		const int	isdst = nowp->dstflag ;
		const int	zoff = nowp->timezone ;
	        pip->open.envdate = TRUE ;
		rs = dater_settimezon(dp,t,zoff,zn,isdst) ;
	    }
	}

	return rs ;
}
/* end subroutine (progmsgenv_beginer) */



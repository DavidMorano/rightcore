/* prognewsrc */

/* prog-newsrc */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_DEBUG	0		/* run-time debug print-outs */


/* revision history:

	= 1995-05-01, David A­D­ Morano
        This code module was completely rewritten to replace any original
        garbage that was here before.

	= 1998-02-10, David A­D­ Morano
        Slightly modified to isolate all of the BBNEWSRC code into confined
        spaces.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        We have isolated the BBNEWSRC code (so that it can be more easily
        changed-modernized) in the future!


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<vecpstr.h>
#include	<field.h>
#include	<dater.h>
#include	<tmtime.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"mkdirlist.h"
#include	"bbnewsrc.h"


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


int prognewsrc(PROGINFO *pip,MKDIRLIST *dlp,cchar ufname[])
{
	BBNEWSRC	ugs ;
	BBNEWSRC_ENT	ue ;
	int		rs ;
	int		c = 0 ;

	if (pip == NULL) return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("prognewsrc: uf=%s\n",ufname) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    int	rs1 = mkdirlist_audit(dlp) ;
	    debugprintf("prognewsrc: mkdirlist_audit() rs=%d\n",rs1) ;
	}
#endif

	if ((rs = bbnewsrc_open(&ugs,ufname,0)) >= 0) {
	    time_t	utime ;
	    int		f_sub ;
	    const char	*ung ;

	    while ((rs = bbnewsrc_read(&ugs,&ue)) > 0) {

		utime = ue.mtime ;
		f_sub = ue.f_subscribed ;
		ung = ue.ngname ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("prognewsrc: ung=%s\n",ung) ;
	    debugprintf("prognewsrc: f_sub=%u\n",f_sub) ;
	}
#endif

		rs = mkdirlist_ung(dlp,ung,utime,f_sub,c) ;
		c += rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("prognewsrc: mkdirlist_ung() rs=%d\n",rs) ;
#endif

		if (rs < 0) break ;
	    } /* end while (reading) */

	    bbnewsrc_close(&ugs) ;
	} /* end if (bbnewsrc) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("prognewsrc: ret rs=%d c=%d\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (prognewsrc) */



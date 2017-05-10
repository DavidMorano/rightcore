/* bprintcleanline */

/* print a clean (cleaned up) line of text */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_NBSP		1		/* fill with NBSP */


/* revision history:

	= 1998-02-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine prints out a cleaned up line of text.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<netdb.h>
#include	<time.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	isprintlatin(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strnnlen(const char *,int,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif /* CF_DEBUGS */

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strwcpyuc(char *,const char *,int) ;
extern char	*strcpylc(char *,const char *) ;
extern char	*strcpyuc(char *,const char *) ;


/* external variables */


/* local structures */


/* forward references */

#if	CF_NBSP
static int	clean_nbsp(char *,int) ;
#else
static int	clean_del(char *,int) ;
#endif

static int	ischarok(int) ;
static int	isend(int) ;


/* module global variables */


/* local variables */


/* exported subroutines */


int bprintcleanline(ofp,lp,ll)
bfile		*ofp ;
char		lp[] ;
int		ll ;
{
	int	rs = SR_OK ;
	int	oli = 0 ;
	int	f_needeol = FALSE ;


	if (ofp == NULL) return SR_FAULT ;

	if (ofp->magic != BFILE_MAGIC) return SR_NOTOPEN ;

	if (ofp->f.nullfile) goto ret0 ;

/* continue */

	if (lp == NULL)
	    return SR_FAULT ;

	if (ll < 0)
	    ll = strlen(lp) ;

	while ((ll > 0) && isend(lp[ll - 1])) {
	    ll -= 1 ;
	}

#if	CF_DEBUGS
	debugprintf("bprintcleanline: ll=%u lp=>%t<¬\n",ll,
		lp,strlinelen(lp,ll,40)) ;
#endif 

#if	CF_NBSP
	oli = clean_nbsp(lp,ll) ;
#else
	oli = clean_del(lp,ll) ;
#endif

	if ((oli == 0) || (lp[oli - 1] != '\n'))
	    f_needeol = TRUE ;

	if ((rs >= 0) && (oli > 0))
	    rs = bwrite(ofp,lp,oli) ;

	if ((rs >= 0) && f_needeol) {
	    oli += 1 ;
	    rs = bputc(ofp,'\n') ;
	}

ret0:

#if	CF_DEBUGS
	debugprintf("bprintcleanline: ret rs=%d oli=%u\n",
		rs,oli) ;
#endif /* CF_DEBUGS */

	return (rs >= 0) ? oli : rs ;
}
/* end subroutine (bprintcleanline) */


/* local subroutines */


#if	CF_NBSP

static int clean_nbsp(lp,ll)
char		*lp ;
int		ll ;
{
	int	ili ;
	int	ch ;
	int	f ;


	for (ili = 0 ; ili < ll ; ili += 1) {

	    ch = (lp[ili] & 0xff) ;
	    f = isprintlatin(ch) || ischarok(ch) ;
	    if (! f)
	        lp[ili] = (char) CH_NBSP ;

	} /* end for */

	return ili ;
}
/* end subroutine (clean_nbsp) */

#else /* CF_NBSP */

static int clean_del(lp,ll)
char		*lp ;
int		ll ;
{
	int	ili, oli ;
	int	ch ;
	int	f_flipped = FALSE ;
	int	f ;


	oli = 0 ;
	for (ili = 0 ; ili < ll ; ili += 1) {

	    ch = (lp[ili] & 0xff) ;
	    f = isprintlatin(ch) || ischarok(ch) ;
	    if (f) {

	        if (f_flipped) {
	            lp[oli++] = lp[ili] ;
	        } else
	            oli += 1 ;

	    } else 
	        f_flipped = TRUE ;

	} /* end for */

	return oli ;
}
/* end subroutine (cleandel) */

#endif /* CF_NBSP */


static int ischarok(ch)
int	ch ;
{
	int	f ;


	f = (ch == '\t') || (ch == '\n') ;

	return f ;
}
/* end subroutine (ischarok) */


static int isend(ch)
int	ch ;
{
	int	f ;


	f = (ch == '\n') || (ch == '\r') ;

	return f ;
}
/* end subroutine (isend) */




/* mailmsgmatenv */

/* manipulate an UNIX® message envelope */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_SAFE		0		/* "safe" mode */
#define	CF_SPACETAB	1		/* use space-tab as white space */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine detects if a mail-message "environment" line is present
        in the caller-supplied bufer.

        Note: The following strings are considered critical to parsing and
        otherwise understanding the envelope format. These are:

		"From"
		"remote from"
		"forwarded by"

        Currently, we do not allow any whitespace after the initial "From" other
        than a SPACE character; and many other implementations only consider
        "From " to be a valid introducer. Producers of envelopes should consider
        always putting a SPACE character there rather than some other whitespace
        character.

	What is a proper representative (example) envelope format?

	> From rightcore.com!dam Wed Dec 8 11:44:30 EDT 1993 -0400

	What are the optional parts?

	From rightcore.com!dam Wed Dec 8 11:44[:30] [EDT] [[19]93] [-0400]


	Description:

	Check it the given test string is the start of a mail message.

	Synopsis:

	int mailmsgmatenv(mep,mbuf,mlen)
	MAILMSGMATENV	*mep ;
	char		mbuf[] ;
	int		mlen ;

	Arguments:

	mep		pointer to message environment information
	mbuf		buffer holding data to be checked
	mlen		length of buffer to check

	Returns:

	>=0		match and this is the length of the address-part
	<0		error or no match


*******************************************************************************/


#define	MAILMSGMATENV_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<string.h>

#include	<vsystem.h>
#include	<char.h>
#include	<tmstrs.h>
#include	<localmisc.h>

#include	"mailmsgmatenv.h"


/* local defines */

#define	SPACETAB(c)	(((c) == ' ') || ((c) == '\t'))

#if	CF_SPACETAB
#define	HEADERWHITE(c)	SPACETAB(c)
#else
#define	HEADERWHITE(c)	CHAR_ISWHITE(c)
#endif


/* external subroutines */

extern int	sichr(cchar *,int,int) ;
extern int	sisub(cchar *,int,cchar *) ;
extern int	sfshrink(cchar *,int,cchar **) ;
extern int	sfnext(cchar *,int,cchar **) ;
extern int	matcasesub(cchar **,cchar *,int) ;
extern int	hasalldig(cchar *,int) ;
extern int	isdigitlatin(int) ;
extern int	isEOL(int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	mailmsgmatenv_ema(MAILMSGMATENV *,cchar *,int) ;
static int	mailmsgmatenv_date(MAILMSGMATENV *,cchar *,int) ;
static int	mailmsgmatenv_datefin(MAILMSGMATENV *,cchar *,int) ;
static int	mailmsgmatenv_remote(MAILMSGMATENV *,cchar *,int) ;


/* local variables */

static cchar	*exts[] = {
	"remote from",
	"forwarded by",
	NULL
} ;


/* exported subroutines */


int mailmsgmatenv(MAILMSGMATENV *mep,cchar *sp,int sl)
{
	int		len = 0 ;
	int		f_start = TRUE ;

#if	CF_SAFE
	if (mep == NULL) return SR_FAULT ;
	if (sp == NULL) return SR_FAULT ;
#endif /* CF_SAFE */

	if (sl < 0) sl = strlen(sp) ;

	while (sl && isEOL(sp[sl-1])) sl -= 1 ;

#if	CF_DEBUGS
	debugprintf("mailmsgmatenv: ent> %t\n",sp,strnlen(sp,sl)) ;
#endif

	if ((sl > 0) && (*sp == '>')) {
	    sp += 1 ;
	    sl -= 1 ;
	    f_start = FALSE ;
	}

	if ((sl > 5) && (sp[0] == 'F')) {
	    int	si = 5 ;
	    if (strncmp("From ",sp,si) == 0) {
		memset(mep,0,sizeof(MAILMSGMATENV)) ;
	        mep->rt = -1 ;
	        sp += si ;
	        sl -= si ;
		if ((si = mailmsgmatenv_ema(mep,sp,sl)) > 0) {
	            sp += si ;
	            sl -= si ;
		    if ((si = mailmsgmatenv_date(mep,sp,sl)) > 0) {
	                sp += si ;
	                sl -= si ;
			if ((si = mailmsgmatenv_remote(mep,sp,sl)) >= 0) {
			    len = mep->a.el ;
			    mep->f.start = f_start ;
			}
		    }
		}
	    }
	}

#if	CF_DEBUGS
	if (mep->a.ep != NULL) {
	debugprintf("mailmsgmatenv: al=%d a=>%t<\n",
		mep->a.el,mep->a.ep,mep->a.el) ;
	}
	if (mep->d.ep != NULL) {
	debugprintf("mailmsgmatenv: dl=%d d=>%t<\n",
		mep->d.el,mep->d.ep,mep->d.el) ;
	}
	if (mep->r.ep != NULL) {
	debugprintf("mailmsgmatenv: rl=%d r=>%t<\n",
		mep->r.el,mep->r.ep,mep->r.el) ;
	}
	debugprintf("mailmsgmatenv: ret len=%d\n",len) ;
#endif

	return len ;
}
/* end subroutine (mailmsgmatenv) */


/* local subroutines */


static int mailmsgmatenv_ema(MAILMSGMATENV *mep,cchar *sp,int sl)
{
	int		skip = 0 ;
	int		cl ;
	cchar		*cp ;
	if ((cl = sfnext(sp,sl,&cp)) > 0) {
	    mep->a.ep = cp ;
	    mep->a.el = cl ;
	    skip = ((cp+cl)-sp) ;
	}
	return skip ;
}
/* end subroutine (mailmsgmatenv_ema) */


/* > From rightcore.com!dam Wed Dec 8 11:44:30 EDT 1993 -0400 */
static int mailmsgmatenv_date(MAILMSGMATENV *mep,cchar *sp,int sl)
{
	int		skip = 0 ;
	int		cl ;
	cchar		*cp ;
	if ((cl = sfnext(sp,sl,&cp)) > 0) {
	    int		rl ;
	    cchar	*begin = cp ;
	    cchar	*rp ;
	    if (tmstrsday(cp,cl) >= 0) {
		int	si = ((cp+cl)-sp) ;
		rl = (sl-si) ;
		rp = (sp+si) ;
	        mep->d.ep = begin ;
		mep->d.el = ((sp+sl)-begin) ;
	        if ((cl = sfnext(rp,rl,&cp)) > 0) {
		    if (tmstrsmonth(cp,cl) >= 0) {
		        si = ((cp+cl)-sp) ;
		        rp = (sp+si) ;
		        rl = (sl-si) ;
	        	if ((cl = sfnext(rp,rl,&cp)) > 0) {
			    if (hasalldig(cp,cl)) {
		                si = ((cp+cl)-sp) ;
		                rp = (sp+si) ;
		                rl = (sl-si) ;
				if ((si = sichr(rp,rl,':')) >= 0) {
			    	    skip = sl ;
		            	    rp += (si+1) ;
		            	    rl -= (si+1) ;
				    si = mailmsgmatenv_datefin(mep,rp,rl) ;
				    if (si >= 0) {
					skip = (rp+si-begin) ;
					mep->d.el = (rp+si-begin) ;
				    }
				}
			    }
			}
		    }
		}
	    }
	}
	return skip ;
}
/* end subroutine (mailmsgmatenv_date) */


static int mailmsgmatenv_datefin(MAILMSGMATENV *mep,cchar *rp,int rl)
{
	int		i ;
	int		si = -1 ;
	for (i = 0 ; exts[i] != NULL ; i += 1) {
	    if ((si = sisub(rp,rl,exts[i])) >= 0) break ;
	} /* end for */
	if (si >= 0) {
	    mep->rt = i ;
	}
	return si ;
}
/* end subroutine (mailmsgmatenv_datefin) */


static int mailmsgmatenv_remote(MAILMSGMATENV *mep,cchar *sp,int sl)
{
	int		skip = 0 ;
	if ((sl > 0) && (mep->rt >= 0)) {
	    const int	el = strlen(exts[mep->rt]) ;
	    int		cl ;
	    cchar	*cp ;
	    sp += el ;
	    sl -= el ;
	    if ((cl = sfnext(sp,sl,&cp)) > 0) {
		mep->r.ep = cp ;
		mep->r.el = cl ;
		skip = ((cp+cl)-sp) ;
	    }
	}
	return skip ;
}
/* end subroutine (mailmsgmatenv_remote) */



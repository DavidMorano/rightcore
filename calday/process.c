/* process */

/* process a file */


#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that
	did similar types of functions.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine reads the file and prints out any matching
	entries.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<field.h>
#include	<char.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"
#include	"sfill.h"



/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	OLINELEN	76

#define	NBLANKS		40



/* external subroutines */

extern int	nextfield(const char *,int,const char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	bwriteblanks(bfile *,int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;


/* external variables */


/* local structures */

struct datespec {
	int	month, day ;
} ;


/* forward references */

static int	nexttoken(const char *,int,char **) ;
static int	parsedate(struct proginfo *,struct datespec *,
			const char *,int) ;


/* local variables */

static const char	blanks[] = "                                        " ;

static const unsigned char	wterms[] = {
	0x00, 0x3F, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;

static const char	*days[] = {
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
	NULL
} ;

static const char	*whens[] = {
	"First",
	"Second",
	"Third",
	"Fourth",
	"Fifth",
	"Last",
	NULL
} ;


/* exported subroutines */


int process(pip,ofp,name,namelen)
struct proginfo	*pip ;
bfile		*ofp ;
const char	name[] ;
int		namelen ;
{
	struct datespec	ds ;

	FIELD	fsb ;

	SFILL	filler ;

	bfile	cfile ;

	int	rs, rs1 ;
	int	ml, sl, cl, ll ;
	int	olinelen = OLINELEN ;
	int	c = 0 ;
	int	f_bol, f_eol ;
	int	f_continue ;
	int	f_lead ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	linebuf[LINEBUFLEN + 1] ;
	char	*np ;
	char	*tp, *sp, *cp, *lp ;


	if (name == NULL) 
		return SR_FAULT ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
		debugprintf("process: entered name=%s\n",name) ;
#endif

	np = (char *) name ;
	if (namelen >= 0) {
		ml = MIN(MAXPATHLEN,namelen) ;
		strwcpy(tmpfname,name,ml) ;
	}

	if (strcmp(name,"-") != 0)
	rs = bopen(&cfile,np,"r",0666) ;

	else
	rs = bopen(&cfile,BFILE_STDIN,"dr",0666) ;

	if (rs < 0)
		goto ret0 ;

	rs = sfill_start(&filler) ;
	if (rs < 0)
		goto ret1 ;

	f_bol = TRUE ;
	while ((rs = breadline(&cfile,linebuf,LINEBUFLEN)) > 0) {

		lp = linebuf ;
		ll = rs ;

		if (linebuf[ll - 1] == '\n') {
			f_eol = TRUE ;
			ll -= 1 ;
		}

		if (ll == 0) continue ;

		f_continue = f_bol && CHAR_ISWHITE(lp[0]) ;

		if ((rs = field_start(&fsb,lp,ll)) >= 0) {

		int	fl ;

		char	*fp ;


		if (! f_continue) {

			while (TRUE) {

			rs = sfill_wline(&filler,ofp,olinelen) ;
			if (rs <= 0)
				break ;

			} /* end while */

/* get a new date specification */

			fl = field_get(&fsb,wterms,&fp) ;

			rs1 = parsedate(pip,&ds,fp,fl) ;

		} /* end if (not a continuation) */

		while ((fl = field_get(&fsb,wterms,&fp)) >= 0) {

	        	rs = sfill_proc(&filler,ofp,olinelen,fp,fl) ;
			if (rs < 0)
				break ;

			if (fsb.term == '#')
				break ;

		} /* end while */

		field_finish(&fsb) ;

	    } /* end if */

	    if (rs < 0)
		break ;

	    f_bol = f_eol ;

	} /* end while */

	if (rs >= 0)
		while ((rs = sfill_wline(&filler,ofp,olinelen)) > 0) ;

ret2:
	sfill_finish(&filler) ;

ret1:
	bclose(&cfile) ;

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (process) */


/* local subroutines */


static int parsedate(pip,dsp,fbuf,flen)
struct proginfo	*pip ;
struct datespec	*dsp ;
const char	fbuf[] ;
int		flen ;
{
	int	rs ;
	int	i, j ;
	int	sl, cl ;

	const char	*tp, *sp, *cp ;


	if (flen < 0)
		flen = strlen(fbuf) ;

	tp = strnchr(fbuf,flen,'/') ;

	if (tp == NULL)
		return SR_INVALID ;

	cp = fbuf ;
	cl = (tp - fbuf) ;
	rs = cfdeci(cp,cl,&dsp->month) ;

	if (rs >= 0) {

		rs = SR_INVALID ;
		sp = (tp + 1) ;
		sl = (fbuf + flen) - tp ;
		cl = nextfield(sp,sl,&cp) ;

		if (cl && isdigit(cp[0])) {

			rs = cfdeci(cp,cl,&dsp->day) ;

		} else if (cl >= 3) {

		i = matstr(days,cp,3) ;

		if ((i >= 0) && (cl > 3)) {

		j = matostr(days,3,(cp + 3),(cl - 3)) ;

		if (j >= 0) {

			rs = whatday(pip,i,j,&dsp->day) ;

		}

		}

		} 

	}

	return rs ;
}
/* end subroutine (parsedate) */


static int nexttoken(sp,sl,rpp)
const char	sp[] ;
int		sl ;
char		**rpp ;
{


	if (sl <= 0)
	    return 0 ;

	*rpp = (char *) sp ;
	if (! CHAR_ISWHITE(*sp)) {

	    while ((sl > 0) && (! CHAR_ISWHITE(*sp))) {
	        sp += 1 ;
	        sl -= 1 ;
	    }

	} else
	    sp += 1 ;

	return (sp - (*rpp)) ;
}
/* end subroutine (nexttoken) */


#ifdef	COMMENT

	    {
	        int	olenr, n ;
	        int	ill ;
	        int	f_leading ;


	        olenr = pip->linewidth ;
	        while (llen > 0) {

	            cl = nexttoken(lp,llen,&cp) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("process: tl=%u token=>%t<\n",cl,cp,cl) ;
#endif

	            f_leading = FALSE ;
	            ill = cl ;
	            if (cl > olenr) {

	                f = ((! opts.rmwhite) || (! CHAR_ISWHITE(*cp))) ;

	                if (f) {

	                    f_leading = TRUE ;
	                    rs = bputc(ofp,'\n') ;

	                    wlen += rs ;

	                    if (rs >= 0)
	                        rs = bwriteblanks(ofp,pip->indent) ;

	                    wlen += rs ;
	                    li = pip->indent ;
	                    olenr = pip->linewidth - pip->indent ;

	                } else
	                    cl = 0 ;

	            } /* end if (handling line overflow) */

	            if (rs >= 0) {

	                if (cl > 0) {

#ifdef	COMMENT
	                    if (opts.rmwhite && f_leading) {

	                        while ((cl > 0) && CHAR_ISWHITE(*cp)) {
	                            cp += 1 ;
	                            cl -= 1 ;
	                        }

	                    }
#endif /* COMMENT */

	                    rs = bwrite(ofp,cp,cl) ;

	                    wlen += rs ;

	                }

	                if (cl > 0) {

	                    if (*cp == '\t') {

	                        n = uceil((li % TABLEN),TABLEN) ;

	                        li += n ;
	                        olenr -= n ;

	                    } else {
	                        li += cl ;
	                        olenr -= cl ;
	                    }

	                } /* end if (handling tab expansion) */

	            } /* end if (no error from writing leader) */

	            lp += ill ;
	            llen -= ill ;

	            if (rs < 0)
	                break ;

	        } /* end while (processing tokens) */

	        rs = bputc(ofp,'\n') ;

	        wlen += rs ;

	    } /* end block */

#endif /* COMMENT */




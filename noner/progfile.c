/* progfile */

/* process a file */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* time-time debug print-outs */
#define	CF_AGECHECK	0		/* age-check */


/* revision history:

	= 1996-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that did similar
	types of functions.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine provides the actual check and fix on the files
	specified.


******************************************************************************/


#include	<envstandards.h>

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
#include	<paramopt.h>
#include	<vecstr.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	SUFBUFLEN
#define	SUFBUFLEN		MAXNAMELEN
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matcasestr(const char **,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	isprintlatin(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnrchr(const char *,int,int) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */

struct echars {
	uint	latin : 1 ;
	uint	ff : 1 ;
	uint	cr : 1 ;
	uint	bell : 1 ;
	uint	bs : 1 ;
} ;


/* forward references */

static int	proccheck(struct proginfo *,const char *) ;
static int	isourprint(struct echars,int) ;
static int	isprintreg(int) ;


/* local variables */


/* exported subroutines */


int progfile(pip,pop,fname)
struct proginfo	*pip ;
PARAMOPT	*pop ;
const char	fname[] ;
{
	bfile	infile ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	bnl ;
	int	rc = 0 ;
	int	f_skipfile = FALSE ;

	char	timebuf[TIMEBUFLEN + 1] ;
	char	*bnp ;
	char	*tp ;


	if (fname == NULL)
	    return SR_FAULT ;

	if (fname[0] == '\0')
	    return SR_INVALID ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2)) {
	    debugprintf("progfile: fname=%s\n",fname) ;
	    debugprintf("progfile: progmode=%u\n",pip->progmode) ;
	    debugprintf("progfile: suffix=%u\n",pip->f.suffix) ;
	}
#endif /* CF_DEBUG */

	pip->c_total += 1 ;

/* check if this file has the correct file-suffix */

	f_skipfile = FALSE ;
	if (pip->f.suffix) {

	    bnl = sfbasename(fname,-1,&bnp) ;

	    tp = strnrchr(bnp,bnl,'.') ;

	    if (tp != NULL) {

	        rs1 = vecstr_find(&pip->suffixes,(tp + 1)) ;
	        if (rs1 < 0)
	            f_skipfile = TRUE ;

	    } else
	        f_skipfile = TRUE ;

	} /* end if (suffix check) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfile: MIDDLE f_skipfile=%u\n",f_skipfile) ;
#endif

	if (! f_skipfile) {

	    struct ustat	sb ;


	    rs1 = u_stat(fname,&sb) ;

	    if ((rs1 < 0) || (! S_ISREG(sb.st_mode)))
	        f_skipfile = TRUE ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("progfile: ageint=%u \n",
	            pip->ageint) ;
	        debugprintf("progfile: mtime=%s\n",
	            timestr_logz(sb.st_mtime,timebuf)) ;
	    }
#endif /* CF_DEBUG */

#if	CF_AGECHECK
	    if (! f_skipfile) {
	        if ((pip->daytime - sb.st_mtime) < pip->ageint)
	            f_skipfile = TRUE ;
	    }
#endif /* CF_AGECHECK */

	} /* end if (age check) */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfile: FINAL f_skipfile=%u\n",f_skipfile) ;
#endif

	if (! f_skipfile) {

/* process this file */

	    pip->c_scanned += 1 ;
	    rs = proccheck(pip,fname) ;
	    rc = (rs > 0) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("progfile: proccheck() rs=%d\n",rs) ;
#endif

	} /* end if (processing file) */

/* should we put out an informational message? */

	if (pip->debuglevel > 0)
	    bprintf(pip->efp,"%s: file=%s (%d)\n",
	        pip->progname, fname,rc) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progfile: ret rs=%d rc=%u\n",rs,rc) ;
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (progfile) */


/* local subroutines */


static int proccheck(pip,fname)
struct proginfo	*pip ;
const char	fname[] ;
{
	struct echars	ec ;

	bfile	cfile, *cfp = &cfile ;

	uint	ch ;

	int	rs ;
	int	len ;
	int	line = 0 ;
	int	i = 0 ;
	int	f_none = FALSE ;
	int	f ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	*kp ;


	memset(&ec,0,sizeof(struct echars)) ;
	ec.latin = pip->f.latin ;
	ec.ff = pip->f.formfeed ;
	ec.cr = pip->f.carriage ;
	ec.bell = pip->f.bell ;
	ec.bs = pip->f.backspace ;

	if ((fname[0] != '\0') && (fname[0] != '-'))
	    rs = bopen(cfp,fname,"r",0666) ;

	else
	    rs = bopen(cfp,BFILE_STDIN,"dr",0666) ;

	if (rs < 0)
	    goto ret0 ;

	while ((rs = breadline(cfp,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    if (linebuf[len-1] == '\n')
	        len -= 1 ;

	    for (i = 0 ; i < len ; i += 1) {
	        ch = linebuf[i] ;
	        f = isourprint(ec,ch) ;
	        f_none = (! f) ;
	        if (f_none)
	            break ;
	    } /* end for */

	    if (f_none) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            uint ch = (isprintlatin(linebuf[i])) ? linebuf[i] : ' ' ;
	            debugprintf("progfile/proccheck: NONE i=%u ch=>%c< %02X\n",
	                i,ch,linebuf[i]) ;
	        }
#endif

	        break ;
	    }

	    line += 1 ;

	} /* end while (reading lines) */

	bclose(cfp) ;

	if ((rs >= 0) && f_none && (pip->verboselevel >= 1)) {

	    if (pip->verboselevel >= 2)
	        bprintf(pip->ofp,"%s %u:%u\n",fname,(line+1),i) ;

	    else
	        bprintf(pip->ofp,"%s\n",fname) ;

	} /* end if (report) */

ret0:
	return (rs >= 0) ? f_none : rs ;
}
/* end subroutine (proccheck) */


static int isourprint(ec,c)
struct echars	ec ;
int	c ;
{
	int	f ;


	f = (ec.latin) ? isprintlatin(c) : isprintreg(c) ;
	f = f || (ec.ff && (c == CH_FF)) ;
	f = f || (ec.cr && (c == CH_CR)) ;
	f = f || (ec.bell && (c == CH_BELL)) ;
	f = f || (ec.bs && (c == CH_BS)) ;
	return f ;
}
/* end subroutines (isourprint) */


static int isprintreg(c)
int		c ;
{
	int	f ;


	f = isprint(c) ;
	f = f || (c == '\t') ;
	return f ;
}
/* end subroutine (isprintreg) */




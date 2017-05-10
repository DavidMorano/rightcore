/* procfile */

/* process an input file to the output */


#define	CF_DEBUG	1
#define	F_FONT		1		/* change the font INSIDE display */
#define	F_DSBLOCK	0		/* doesn't work ! */


/* revistion history :

	= 87/09/10, David A­D­ Morano

	This subroutine was originally written.


*/


/*******************************************************************

	This subroutine processes a file by writing its contents out
	to an output file with the correct pagination.


*********************************************************************/


#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<baops.h>
#include	<ascii.h>
#include	<char.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#ifndef	LINELEN
#define	LINELEN		200
#endif

#ifndef	BUFLEN
#define	BUFLEN		(LINELEN + LINELEN)
#endif



/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;

extern char	*strnchr(const char *,int,int) ;


/* forward references */


/* local structures */


/* local variables */


/* exported subroutines */


int procfile(pip,fname,fn,f_eject)
struct proginfo	*pip ;
const char	fname[] ;
int		fn ;
int		f_eject ;
{
	struct ustat	sb ;

	bfile	infile, *ifp = &infile ;

	int	rs ;
	int	i ;
	int	c ;
	int	line, len ;
	int	sl, cl ;
	int	f_stdinput ;
	int	f_exit ;

	const char	*sp, *cp, *tp ;

	char	linebuf[LINELEN + 1], *lbp ;
	char	timebuf[TIMEBUFLEN + 1] ;


#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("procfile: fname=%s\n",fname) ;
#endif

/* check the arguments */

	    f_stdinput = TRUE ;
	if ((fname != NULL) && (fname[0] != '-')) {

	f_stdinput = FALSE ;
		rs = u_stat(fname,&sb) ;

		if ((rs >= 0) && S_ISDIR(sb.st_mode))
		rs = SR_ISDIR ;

	if (rs >= 0)
	    rs = bopen(ifp,fname,"r",0666) ;

	} else
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("procfile: bopen() rs=%d\n",rs) ;
#endif

	if (rs < 0)
	    goto ret0 ;

/* output the pages */

	c = 0 ;
	line = 0 ;
	f_exit = FALSE ;
	while (! f_exit) {

/* skip leading blank lines */

	    while ((rs = breadline(ifp,linebuf,LINELEN)) > 0) {

		len = rs ;
	        sl = sfshrink(linebuf,len,&sp) ;

	        if ((sl > 0) && (sp[0] != '%'))
	            break ;

	    } /* end while (skipping leading blank lines) */

	    if (rs <= 0)
	        break ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("procfile: line=>%t<\n",sp,sl) ;
#endif

/* put out the stuff for this next cookie */

	    c += 1 ;
	    bprintf(pip->ofp,".SP\n") ;

#if	F_DSBLOCK
	    bprintf(pip->ofp,".DS CB F \\\\n(aI\n") ;
#else
	    bprintf(pip->ofp,".QS\n") ;
#endif

/* copy over the first line */

	    bprintf(pip->ofp,"%t\n",sp,sl) ;

/* copy over the lines until we reach the author line (if there is one) */

	    sl = 0 ;
	    while ((rs = breadline(ifp,linebuf,LINELEN)) > 0) {

	        len = rs ;
	        if (linebuf[len - 1] == '\n')
			len -= 1 ;

		linebuf[len] = '\0' ;
		sp = linebuf ;
		sl = len ;
		while ((sl > 0) && CHAR_ISWHITE(sp[sl - 1]))
			sl -= 1 ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("procfile: copy line=>%t<\n",sp,sl) ;
#endif

		if ((sl > 0) &&
	        	((sp[0] == '%') || (strncmp(sp,"\t\t--",4) == 0)))
	            break ;

	        rs = bprintf(pip->ofp,"%t\n",sp,sl) ;

		if (rs < 0)
			break ;

	    } /* end while (skipping blank lines) */

	    if (rs <= 0)
	        f_exit = TRUE ;

/* do we have an "author" line ? */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("procfile: AC line=>%t<\n",sp,sl) ;
#endif

	    if ((sl > 0) && (strncmp(sp,"\t\t--",4) == 0)) {

	        bprintf(pip->ofp,".br\n") ;

	        bprintf(pip->ofp,".in +5\n") ;

	        sp += 4 ;
	        sl -= 4 ;
	        while ((sl > 0) && CHAR_ISWHITE(*sp)) {
	            sp += 1 ;
	            sl -= 1 ;
	        }

	        if ((tp = strnchr(sp,sl,CH_LPAREN)) != NULL) {

	            bprintf(pip->ofp,"-- %t\n",sp,(tp - sp)) ;

	            bprintf(pip->ofp,".br\n") ;

	            cp = (tp + 1) ;
	            cl = ((sp + sl) - (tp + 1)) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	debugprintf("procfile: rest A line=>%t<\n",cp,cl) ;
#endif

	            tp = strnchr(cp,cl,CH_RPAREN) ;

	            if (tp != NULL) {
	                *tp = '\0' ;
			cl = tp - cp ;
		    }

	            bprintf(pip->ofp,"%t\n",cp,cl) ;

/* read lines until something makes us break out */

	            while ((rs = breadline(ifp,linebuf,LINELEN)) > 0) {

	                len = rs ;
	                sl = sfshrink(linebuf,len,&sp) ;

	                if ((sl == 0) || (sp[0] == '%'))
	                    break ;

	                tp = strnchr(sp,sl,CH_RPAREN) ;

	                if (tp != NULL) {
	                    *tp = '\0' ;
			    sl = tp - sp ;
		        }

	                rs = bprintf(pip->ofp,"%t\n",sp,sl) ;

			if (rs < 0)
				break ;

	            } /* end while */

	        } else
	            bprintf(pip->ofp,"-- %t\n",sp,sl) ;

	        bprintf(pip->ofp,".in -5\n") ;

	    } /* end if (had an author) */

/* finish the last c */

#if	F_DSBLOCK
	    bprintf(pip->ofp,".DE\n") ;
#else
	    bprintf(pip->ofp,".QE\n") ;
#endif

		if (rs < 0)
			break ;

	} /* end while (reading lines) */

done:
	bclose(ifp) ;

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procfile) */




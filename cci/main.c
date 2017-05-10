/* main */

/* program to correct indentations */


/* revision history:

	= 1985-02-12, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine is a little program that is used to follow
	the processing of a C language source file by the CB program.


******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	sfbasename(const char *,int,const char **) ;


/* forward references */

static int	procfile(struct proginfo *,bfile *,const char *) ;


/* local variables */


/* exported subroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	bfile	errfile ;
	bfile	outfile, *ofp = &outfile ;

	int	rs ;
	int	i ;
	int	ex = EX_INFO ;

	const char	*cp ;


	memset(pip,0,sizeof(struct proginfo)) ;

	sfbasename(argv[0],-1,&cp) ;
	pip->progname = cp ;

	if (bopen(&errfile,BFILE_STDERR,"dwca",0666) >= 0) {

	    pip->efp = &errfile ;
	    bcontrol(&errfile,BC_LINEBUF,0) ;
	}

/* open standard-output */

	rs = bopen(ofp,BFILE_STDOUT,"dwct",0666) ;

	if (rs < 0) {
	    ex = EX_CANTCREAT ;
	    goto badoutopen ;
	}

/* loop processing files */

	for (i = 1 ; argv[i] != NULL ; i += 1) {

	    cp = argv[i] ;
	    if ((cp[0] != '\0') && (strcmp(cp,"--") != 0)) {
	        rs = procfile(pip,ofp,cp) ;
	    }

	        if (rs < 0) break ;
	} /* end for */

ret4:
	ex = (rs >= 0) ? EX_OK : EX_DATAERR ;

ret3:
ret2:
	bclose(ofp) ;

badoutopen:
ret1:
	bclose(pip->efp) ;

ret0:
	return ex ;
}
/* end subroutine (main) */


/* local subroutines */


static int procfile(pip,ofp,fname)
struct proginfo	*pip ;
bfile		*ofp ;
const char	fname[] ;
{
	bfile	infile, *ifp = &infile ;

#ifdef	INSIDE
	int	inside = 0 ;
#endif

	int	rs ;
	int	i, j ;
	int	nblank = 0 ;
	int	llen, outlen ;
	int	c ;
	int	c_white = 0 ;
	int	f_comment = FALSE ;
	int	f_comstart = FALSE ;
	int	f_comend = FALSE ;
	int	f_braceend = FALSE ;
	int	f_leading = FALSE ;

	char	linebuf[LINEBUFLEN + 1], outbuf[LINEBUFLEN * 4] ;


	if (fname == NULL)
	    return SR_FAULT ;

	if ((fname[0] != '-') && (fname[0] != '\0')) {
	    rs = bopen(ifp,fname,"r",0666) ;
	} else
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	if (rs < 0)
	    goto badinopen ;

/* go through the loops */

	while ((rs = breadline(ifp,linebuf,LINEBUFLEN)) > 0) {

	    llen = rs ;
	    outlen = 0 ;
	    for (i = 0 ; i < llen ; i += 1) {

	        c = linebuf[i] ;
	        switch (c) {

/* handle a possible beginning of a comment */
	        case '/':
	            outbuf[outlen++] = c ;
	            f_braceend = FALSE ;

	            if (f_comment) {

	                if (f_comend) {

	                    f_comend = FALSE ;
	                    f_comment = FALSE ;

	                }

	                f_leading = FALSE ;

	            } else {

	                f_comstart = TRUE ;

	            }

	            break ;

	        case '*':
	            f_comend = FALSE ;
	            f_braceend = FALSE ;

	            outbuf[outlen++] = c ;

	            if (! f_comment) {

	                if (f_leading && f_comstart) {

	                    outbuf[0] = '/' ;
	                    outbuf[1] = c ;
	                    outlen = 2 ;

	                }

	            } else
	                f_comend = TRUE ;

	            if (f_comstart)
	                f_comment = TRUE ;

	            f_leading = FALSE ;
	            break ;

	        case '\n':
	            if (i == 0)
	                nblank += 1 ;

	            else if (nblank > 0)
	                nblank -= 1 ;

	            f_leading = FALSE ;
	            f_braceend = FALSE ;
	            f_comstart = FALSE ;
	            f_comend = FALSE ;
	            outbuf[outlen++] = c ;
	            break ;

	        case '\t':
	            f_comstart = FALSE ;
	            f_comend = FALSE ;
	            f_braceend = FALSE ;

	            if (f_comment) {

	                outbuf[outlen++] = c ;

	            } else {

	                if (i == 0) {

	                    outbuf[outlen++] = c ;
	                    f_leading = TRUE ;

	                } else if (f_leading) {

	                    for (j = 0 ; j < 4 ; j += 1)
	                        outbuf[outlen++] = ' ' ;

	                } else 
	                    outbuf[outlen++] = c ;

	            }

	            break ;

	        case CH_LPAREN:
	            f_leading = FALSE ;
	            f_braceend = FALSE ;
	            f_comend = FALSE ;

	            outbuf[outlen++] = c ;

	            if (f_comment)
	                break ;

	            f_comstart = FALSE ;
#ifdef	INSIDE
	            inside += 1 ;
#endif
	            break ;

	        case CH_RPAREN:
	            f_leading = FALSE ;

	            outbuf[outlen++] = c ;

	            if (f_comment)
	                break ;

	            f_comend = FALSE ;
	            f_comstart = FALSE ;
	            f_braceend = TRUE ;
#ifdef	INSIDE
	            inside -= 1 ;
#endif

	            break ;

	        case ';':
	            f_leading = FALSE ;
	            f_comend = FALSE ;
	            f_comstart = FALSE ;

	            if (f_comment) {

	                outbuf[outlen++] = c ;

	            } else {

	                if (f_braceend) {

	                    if (((llen - i) > 0) && (linebuf[i + 1] == ' ')) {

	                        i += 1 ;
	                    }

	                    outbuf[outlen++] = ' ' ;
	                    outbuf[outlen++] = c ;
	                    f_braceend = FALSE ;

	                } else if ((! c_white) && (linebuf[i + 1] == '\n')) {

	                    outbuf[outlen++] = ' ' ;
	                    outbuf[outlen++] = c ;

	                } else
	                    outbuf[outlen++] = c ;

	            }

	            break ;

	        case ' ':
	            c_white = 2 ;

/* fall through to case below */

	        default:
	            outbuf[outlen++] = c ;
	            f_braceend = FALSE ;
	            f_leading = FALSE ;
	            f_comend = FALSE ;
	            f_comstart = FALSE ;
	            break ;

	        } /* end switch */

	        if (c_white)
	            c_white -= 1 ;

	    } /* end for */

	    if (rs >= 0)
	    rs = bwrite(ofp,outbuf,outlen) ;

	    if (rs < 0) break ;
	} /* end while (reading lines) */

	if ((rs >= 0) && (nblank < 2)) {
	    for (i = 0 ; i < (2 - nblank) ; i += 1)
	        bputc(ofp,'\n') ;
	}

	bclose(ofp) ;

badinopen:
	return rs ;
}
/* end subroutine (procfile) */



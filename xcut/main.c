/* main */

/* copy the contents of CUT BUFFER #0 to the standard output */


/* revision history:

	= 1991-09-10, David A­D­ Morano


*/


#define	CF_DEBUGS	0


/*
 * NAME:
 *	xcut
 *
 * USAGE:
 *	xcut
 *
 * WHAT:
 *	This routine copies the cut buffer to standard output.
 *
 * SPECIAL CONSIDERATIONS:
 	Currently, this only reads from cut buffer #0.  The user must use
	the program 'xcutsel' to get access to the "primary" cut buffer.
 */


#include	<sys/types.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<X11/Xlib.h>

#include	<X11/Xutil.h>

#include	<bfile.h>

#include	"localmisc.h"


/* defines */

#define	VERSION		"0"
#define	NPARG		1	/* number of positional arguments */

#define	LINELEN		200
#define	BUFLEN		(LINELEN + LINELEN)


/* * Printer objects.  */

#define LINEFEED	0	/* print a linefeed */
#define FORMFEED	1	/* print a formfeed */
#define PRICUTBUF	2	/* print from the primary cut buffer */


/* external functions */

extern int	cfdec() ;


/* external variables */

	extern int	errno ;




int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	Display		*mydisplay ;	/* pointer to display structure	*/

	bfile		errfile, *efp = &errfile ;
#ifdef	COMMENT
	bfile		outfile, *ofp = &outfile ;
#endif

	int		argl, aol ;
	int		pan, i ;
	int		rs, len ;

	int	f_debug = FALSE ;
	int	f_usage = FALSE ;
	int	f_num = FALSE ;

	char	*progname, *argp, *aop ;
	char	linebuf[LINELEN + 1], *lbp ;
	char	*ofname = NULL ;
	char	*cbuf = ((char *) 0) ;
#ifdef	COMMENT
	char	*cbp ;
#endif

	progname = argv[0] ;
	if (bopen(efp,BERR,"wca",0666) < 0) return BAD ;

	pan = 0 ;			/* number of positional so far */
	i = 1 ;
	argc -= 1 ;

	while (argc > 0) {

	    argp = argv[i++] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

			f_num = TRUE ;

	            } else {

	                aop = argp ;
	                aol = argl ;
	                while (--aol) {

	                    akp += 1 ;
	                    switch ((int) *aop) {

	                    case 'D':
	                        f_debug = TRUE ;
	                        break ;

	                    case 'V':
	                        bprintf(efp,"%s: version %s\n",
	                            progname,VERSION) ;

	                        break ;

	                    default:
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            progname,*aop) ;

	                    case '?':
	                        f_usage = TRUE ;

	                    } /* end switch */

	                } /* end while */
	            }

	        } else {

	            pan += 1 ;	/* stand-alone dash, skip position */

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                if (argl > 0) ofname = argp ;

	                break ;

	            default:
	                break ;
	            }

	            pan += 1 ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",
	                progname) ;

	        }

	    } /* end if */

	} /* end while */

	if (f_debug) 
		bprintf(efp,"finished parsing arguments %d\n",f_num) ;

	if (f_usage) goto usage ;


/* check arguments */

	if (ofname == NULL) ofname = BFILE_STDOUT ;


/* open files */

#ifdef	COMMENT
	rs = bopen(ofp,ofname,"wct",0666) ;

	if (rs < 0) return rs ;
#endif

/* perform initialization processing */

	/* * open a display connection to X server */

	mydisplay = XOpenDisplay((char *) 0) ;

		 /* Fetch the bytes from cut buffer.  */

cbuf = XFetchBuffer(mydisplay,&len,((char *) 0)) ;

if ((cbuf == ((char *) 0)) || (len == 0)) {

    XCloseDisplay(mydisplay) ;

goto badret ;
}

#ifdef	COMMENT
	cbp = cbuf ;
	for (i = 0 ; i < len ; i+= 1) {

	switch ((int) *cbp) {

	} /* end switch */

	}
#endif
	rs = write(1,cbuf,len) ;

XFree(cbuf) ;

    XCloseDisplay(mydisplay) ;

	if (rs != len) goto badwrite ;

done:
#ifdef	COMMENT
	bclose(ofp) ;
#endif

	bclose(efp) ;

	return OK ;

usage:
	bprintf(efp,
	    "usage: %s [-l lines] [-lines] [infile [outfile]]\n",
	    progname) ;

	bprintf(efp,"\t\t[-f font] [-o offset] [-DV]\n") ;

	bprintf(efp,"\t\t[-p point_size] [-v vertical_space]\n") ;

	bprintf(efp,
	    "usage: -l lines    number of lines per page\n") ;

	bprintf(efp,
	    "usage: -lines      number of lines per page\n") ;

	bprintf(efp,
	    "usage: infile      input file\n") ;

	bprintf(efp,
	    "usage: outfile     output file\n") ;

	bprintf(efp,
	    "usage: -f font     TROFF type font specification\n") ;

	bprintf(efp,
	    "usage: offset      left margin shift offset\n") ;

	bprintf(efp,
	    "usage: -D          debugging flag\n") ;

	bprintf(efp,
	    "usage: -V          program version\n") ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad input argument specified\n",progname) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",progname) ;

	goto badret ;

badwrite:
	bprintf(efp,"%s: bad write to standard output\n",progname) ;

badret:
	bclose(efp) ;

	return BAD ;
}



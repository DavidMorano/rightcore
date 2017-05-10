/* main (mkincfile) */

/* make an include file which contains the content of another file */


#define	CF_DEBUGS	0
#define	CF_DEBUG	1


/* revision history:

	= 94/09/10, David A­D­ Morano

	This subroutine was grabbed from someplace as a start.


*/



/*******************************************************************

	This program will read the input file and create a C
	language header "include" file which contains its
	contents in a way that can be used by a C language program.


*********************************************************************/



#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>

#include	<bfile.h>
#include	<exitcodes.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"




/* local defines */

#define	NPARG		2	/* number of positional arguments */

#define	LINELEN		200
#define	BUFLEN		(LINELEN + LINELEN)



/* externals subroutines */

extern char	*strbasename(char *) ;





int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	struct proginfo	pi, *pip = &pi ;

	struct ustat	isb ;

	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;

	int		argl, aol ;
	int		pan, i ;
	int	rs = SR_OK ;
	int	rs1 ;
	int	flen, len ;
	int	ex = EX_INFO ;
	int	f_version = FALSE ;
	int	f_usage = FALSE ;
	int	f_verbose = FALSE ;

	char	*argp, *aop ;
	char	linebuf[LINELEN + 1], *lbp ;
	char	outfnamebuf[MAXNAMELEN + 1] ;
	char	*infname = NULL ;
	char	*outfname = NULL ;
	char	*cp ;


	memset(&pi,0,sizeof(struct proginfo)) ;

	pip->progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"wca",0666) >= 0) {
		pip->efp = efp ;
		bcontrol(efp,BC_LINEBUF,0) ;
	}

	pip->debuglevel = 0 ;

	pan = 0 ;			/* number of positional so far */
	i = 1 ;
	argc -= 1 ;
	while (argc > 0) {

	    argp = argv[i++] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	                aop = argp ;
	                aol = argl ;
	                while (--aol) {

	                    aop += 1 ;
	                    switch ((int) *aop) {

	                    case 'D':
	                        pip->debuglevel = 1 ;
	                        break ;

	                    case 'V':
				f_version = TRUE ;
	                        break ;

	                    case 'i':
	                        if (argc <= 0) 
					goto badargnum ;

	                        argp = argv[i++] ;
	                        argc -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) 
					infname = argp ;

	                        break ;

	                    case '?':
	                        f_usage = TRUE ;
				break ;

	                    case 'v':
	                        f_verbose = TRUE ;
				break ;

	                    default:
				ex = EX_USAGE ;
				f_usage = TRUE ;
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

	                    } /* end switch */

	                } /* end while */

	        } else {

	            pan += 1 ;	/* stand-alone dash, skip position */

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                if (argl > 0) 
				infname = argp ;

	                break ;

	            case 1:
	                if (argl > 0) 
				outfname = argp ;

	                break ;

	            default:
			ex = EX_USAGE ;
	                break ;
	            }

	            pan += 1 ;

	        } else {

			ex = EX_USAGE ;
	            bprintf(efp,"%s: extra arguments ignored\n",
	                pip->progname) ;

	        }

	    } /* end if */

	} /* end while */


	if (pip->debuglevel > 0) 
		bprintf(efp,"%s: finished parsing arguments\n",
			pip->progname) ;


	if (f_version)
	                        bprintf(efp,"%s: version %s\n",
	                            pip->progname,VERSION) ;

	if (f_usage) 
		goto usage ;

	if (f_version)
		goto ret1 ;


/* check arguments */

	if ((infname == NULL) || (infname[0] == '\0')) 
		goto badinfile ;

/* open files */

	if ((rs = bopen(ifp,infname,"r",0666)) < 0) 
		goto badinopen ;

	if (rs = bcontrol(ifp,BC_STAT,&isb) < 0) {

		bclose(ifp) ;

		goto badinopen ;
	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("main: 1\n") ;
#endif

/* what should the name of the output file be ? */

	if ((cp = strchr(infname,'/')) != NULL)
		infname = strbasename(infname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("main: 2\n") ;
#endif

	infname[MAXNAMELEN - 12] = '\0' ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("main: 3\n") ;
#endif

	bufprintf(outfnamebuf,MAXPATHLEN,"incfile_%s.h",infname) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("main: outfname=%s\n",outfnamebuf) ;
#endif

	outfname = outfnamebuf ;
	if ((rs = bopen(ofp,outfname,"wct",0666)) < 0) 
		goto badoutopen ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("main: 5\n") ;
#endif


/* output the header portion */

	bprintf(ofp,"/* %s */\n\n",infname) ;

	bprintf(ofp,"#define	INCFILELEN_%s	%ld\n\n",
		infname,isb.st_size) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("main: 5a\n") ;
#endif

	bprintf(ofp,"unsigned char incfile_%s[%ld + 1] = {\n",
		infname,isb.st_size) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("main: 6\n") ;
#endif

/* process the file */

	flen = 0 ;
	rs1 = 0 ;
	while ((len = breadline(ifp,linebuf,8)) > 0) {

		for (i = 0 ; i < len ; i += 1) {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
			debugprintf("main: 0x%02X,\n",linebuf[i]) ;
#endif

			rs1 = bprintf(ofp,"0x%02X,",linebuf[i]) ;

			if (rs1 < 0)
				break ;

		} /* end for */

		bprintf(ofp,"\n") ;

		flen += len ;

	} /* end while */

	rs = len ;

	bprintf(ofp,"0x00 } ;\n\n") ;


	if (rs < 0)
		bprintf(pip->efp,"%s: error reading input file (rs=%d)\n",
			rs) ;

	if (rs1 < 0)
		bprintf(pip->efp,"%s: error writing output file (rs=%d)\n",
			rs1) ;


	ex = ((rs >= 0) && (rs1 >= 0)) ? EX_OK : EX_DATAERR ;
		

ret3:
	bclose(ofp) ;


	if ((rs >= 0) && f_verbose && 
		(bopen(ofp,BFILE_STDOUT,"wctd",0666) >= 0)) {

		bprintf("output include filename=%s\n",
			outfname) ;

		bprintf("output file length=%d\n",
			flen) ;

		bclose(ofp) ;

	}

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	debugprintf("main: flen=%d\n",flen) ;
#endif


ret2:
	bclose(ifp) ;

earlyret:
ret1:
	bclose(efp) ;

ret0:
	return ex ;

usage:
	bprintf(efp,
	    "%s: USAGE> %s infile\n",
	    pip->progname,pip->progname) ;

	goto ret1 ;

badargunk:
	bprintf(efp,"%s: bad input argument specified\n",
		pip->progname) ;

	goto badarg ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
		pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	goto ret1 ;

badinfile:
	ex = EX_USAGE ;
	bprintf(efp,"%s: no input file was specified\n",
	    pip->progname) ;

	goto ret1 ;

badinopen:
	ex = EX_NOINPUT ;
	bprintf(efp,"%s: could not open input file \"%s\" (rs %d)\n",
	    pip->progname,infname,rs) ;

	goto ret1 ;

badoutopen:
	ex = EX_CANTCREAT ;
	bprintf(efp,"%s: could not open output file \"%s\" (rs %d)\n",
	    pip->progname,outfname,rs) ;

	goto ret2 ;

}
/* end subroutine (main) */




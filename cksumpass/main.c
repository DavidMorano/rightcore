/* main */

/* check summing program */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1998-02-01, David A­D­ Morano
        This subroutine was written for Rightcore Network Services (RNS). The
        program handles very large (greater than 1Gbyte) files without any
        fudging problems.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Compute a checksum (POSIX 'cksum' style) on the data
	passing from input to output.

	Execute as :

	$ cksumpass -s ansfile [input [...]] > outfile


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<cksum.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		1

#define	DEFRECLEN	((BLOCKSIZE * 126) * 10)
#define	MAXRECLEN	((BLOCKSIZE * 128) * 10)


/* external subroutines */

extern int	isdigitlatin(int) ;

extern char	*strbasename(char *) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int main(int argc,cchar **argv,cchar **envv)
{
	PROGINFO	pi, *pip = &pi ;
	CKSUM		sum ;
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;
	uint		sv ;
	int		argr, argl, aol ;
	int		argvalue = -1 ;
	int		rs = SR_OK ;
	int		i, len, npa ;
	int		ifd, ofd = 1 ;
	int		trec_t, trec_f, trec_p ;
	int		bytes, blocks ;
	int		maxnrec = MAXNREC ;
	int		reclen = DEFRECLEN ;
	int		nfile = 0 ;
	int		ex = EX_INFO ;
	int		fd_debug = -1 ;
	int		f_version = FALSE ;
	int		f_verbose = FALSE ;
	int		f_usage = FALSE ;
	int		f_ignore = FALSE ;
	int		f_ignorezero = FALSE ;
	int		f_zero ;

	char	*argp, *aop ;
	char	*pr = NULL ;
	char	*infname = NULL ;
	char	*sumfname = NULL ;
	char	*blockbuf = NULL ;
	char	*reclenp = NULL ;
	char	*cp ;

#if	CF_DEBUGS || CF_DEBUG
	if ((cp = getourenv(envv,VARDEBUGFNAME)) != NULL) {
	    rs = debugopen(cp) ;
	    debugprintf("main: starting DFD=%d\n",rs) ;
	}
#endif /* CF_DEBUGS */

	memset(pip,0,sizeof(PROGINFO)) ;

	pip->progname = strbasename(argv[0]) ;

	if (bopen(&errfile,BFILE_STDERR,"wca",0666) >= 0) {
		pip->efp = &errfile ;
		bcontrol(&errfile,BC_LINEBUF,0) ;
	}

	pip->debuglevel = 0 ;
	pip->f.quiet = FALSE ;

	npa = 0 ;			/* number of positional so far */
	i = 1 ;
	argr = argc - 1 ;
	while ((rs >= 0) && (argr > 0)) {

	    argp = argv[i++] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {
		     const int	ach = MKCHAR(argp[1]) ;

	            if (isdigitlatin(ach)) {

	                if ((rs = cfdec(argp + 1,argl - 1,&maxnrec)) < 0)
	                    goto badarg ;

	                if (maxnrec < 0)
				maxnrec = MAXNREC ;

	            } else {

	                aop = argp ;
	                aol = argl ;
	                while (--aol) {

	                    akp += 1 ;
	                    switch ((int) *aop) {

	                    case 'D':
	                        pip->debuglevel = 2 ;
	                        break ;

	                    case 'V':
	                        f_version = TRUE ;
	                        break ;

/* record length */
	                    case 'b':
	                    case 'r':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[i++] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl > 0) reclenp = argp ;

	                        break ;

	                    case 'q':
	                        pip->f.quiet = TRUE ;
	                        break ;

/* file to receive the cksum answer in */
			case 's':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[i++] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl) sumfname = argp ;

	                        break ;

	                    case 'v':
	                        f_verbose = TRUE ;
	                        break ;

	                    case 'z':
	                        f_ignorezero = TRUE ;
	                        break ;

	                    default:
	                        bprintf(pip->efp,"%s: unknown option - %c\n",
	                            pip->progname,*aop) ;

	                    case '?':
	                        f_usage = TRUE ;

	                    } ; /* end switch */

	                } /* end while */

	            } /* end if */

	        } else {

	            npa += 1 ;	/* increment position count */

	        } /* end if */

	    } else {

	        if (npa < NPARG) {

	            switch (npa) {

	            case 0:
	                if (argl > 0) infname = argp ;

	                break ;

	            default:
	                break ;
	            }

	            npa += 1 ;

	        } else {

			ex = EX_USAGE ;
			f_usage = TRUE ;
	            bprintf(pip->efp,"%s: extra arguments specified\n",
	                pip->progname) ;

	        }

	    } /* end if */

	} /* end while (arguments) */


/* done w/ arguments, now handle miscellaneous */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
		debugprintf("main: finished parsing arguments\n") ;
#endif

	if (f_version) {
	    bprintf(pip->efp,"%s: version %s\n",pip->progname,VERSION) ;
	    goto retearly ;
	}

	if (f_usage) 
		goto usage ;


/* check arguments */

	if (reclenp == NULL) {

	    reclen = DEFRECLEN ;
	    if (pip->debuglevel > 0)
		bprintf(pip->efp,
	        "%s: no record length given, using default %d\n",
	        pip->progname,reclen) ;

	} else {

		int	mf ;


#ifdef	COMMENT
	    l = strlen(reclenp) ;

	    mf = 1 ;
	    if (l > 0) {

	        if (reclenp[l - 1] == 'b') {

	            mf = BLOCKSIZE ;
	            reclenp[l-- - 1] = '\0' ;

	        } else if (reclenp[l - 1] == 'k') {

	            mf = 1024 ;
	            reclenp[l-- - 1] = '\0' ;

	        } else if (tolower(reclenp[l - 1]) == 'm') {

	            mf = 1024 * 1024 ;
	            reclenp[l-- - 1] = '\0' ;

	        }

	    }

	    if (pip->debuglevel > 0) 
		bprintf(pip->efp, "%s: record string \"%s\"\n",
	        pip->progname,reclenp) ;

	    if ((rs = cfdec(reclenp,l,&reclen)) < 0)
	        goto badarg ;

	    reclen = reclen * mf ;
#else
	    if ((rs = cfdecmfi(reclenp,-1,&reclen)) < 0)
	        goto badarg ;

#endif /* COMMENT */

	} /* end if */

	if (reclen > MAXRECLEN) {

	    reclen = MAXRECLEN ;
		if (! pip->f.quiet)
	    bprintf(pip->efp,
		"%s: record length is too large - reduced to %d\n",
	        pip->progname,reclen) ;

	}

	if ((sumfname != NULL) && (sumfname[0] != '\0'))
		bopen(ofp,sumfname,"wct",0666) ;


	if (pip->debuglevel > 0)
		bprintf(pip->efp,"%s: running with record size %d\n",
	    pip->progname,reclen) ;


/* allocate the buffer for the date from the tape */

	if ((blockbuf = valloc(reclen)) == NULL)
		goto badalloc ;


/* open files */

	if (infname != NULL) {
	    if ((ifd = u_open(infname,O_RDONLY,0666)) < 0)
	        goto badinfile ;
	} else 
	    ifd = 0 ;


/* finally go through the loops */

	if (pip->debuglevel > 0)
		bprintf(pip->efp,"%s: about to enter while loop\n",
	    pip->progname) ;

	f_zero = FALSE ;
	trec_f = trec_p = 0 ;
	bytes = blocks = 0 ;


	cksum_start(&sum) ;

	while ((len = u_read(ifd,blockbuf,reclen)) > 0) {

		cksum_accum(&sum,blockbuf,len) ;

		u_write(ofd,blockbuf,len) ;

		if (len == reclen) {
			trec_f += 1 ;
		} else
			trec_p += 1 ;

		bytes += len ;
		if (bytes >= BLOCKSIZE) {

#if	CF_DEBUG
	if (pip->debuglevel > 1)
		debugprintf("main: block increment, bytes=%d\n",bytes) ;
#endif

			blocks += (bytes / BLOCKSIZE) ;
			bytes = bytes % BLOCKSIZE ;

		}

	} /* end while */


#if	CF_DEBUG
	if (pip->debuglevel > 1)
		debugprintf("main: out of loop\n") ;
#endif


	trec_t = trec_f + trec_p ;


	bprintf(ofp,"records\t\tP=%ld F=%ld T=%ld (record size=%db)\n",
	    trec_p,trec_f,trec_t,
		(reclen / BLOCKSIZE)) ;

	bprintf(ofp,"UNIX blocks\t%d remaining bytes %d\n",
	    blocks,bytes) ;

	cksum_getsum(&sum,&sv) ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {

		rs = cksum_getlen(&sum,&len) ;

		debugprintf("main: rs=%d len=%u\n",rs,len) ;
	}
#endif

	bprintf(ofp,"cksum\t\t\\x%08x (%u)\n",sv,sv) ;

	bprintf(ofp,"size\t\t%d Mibytes %d bytes\n",
	    (blocks / 2048),
	    (((blocks % 2048) * BLOCKSIZE) + bytes)) ;


	bclose(ofp) ;


	cksum_finish(&sum) ;


/* finish off */

	u_close(ifd) ;

	u_close(ofd) ;

done:
retearly:
ret1:
	bclose(pip->efp) ;

ret0:
	return ex ;

/* usage */
usage:
	bprintf(pip->efp,
	    "%s: USAGE> %s [-s ansfile] [infile(s) ...]",
	    pip->progname,pip->progname) ;

	bprintf(pip->efp," [-V]\n") ;

	goto badret ;

/* bad arguments */
badargnum:
	bprintf(pip->efp,"%s: not enough arguments specified\n",
	    pip->progname) ;

	goto badarg ;

badarg:
	ex = EX_USAGE ;
	bprintf(pip->efp,"%s: bad argument given (rs %d)\n",
	    pip->progname,rs) ;

	goto ret1 ;

/* other bad */
badalloc:
	bprintf(pip->efp,"%s: could not allocate buffer memory\n",
	    pip->progname) ;

	goto badret ;

badinfile:
	bprintf(pip->efp,"%s: cannot open the input file (rs %d)\n",
	    pip->progname,ifd) ;

	goto badret ;

badoutopen:
	bprintf(pip->efp,"%s: cannot open the output file (rs %d)\n",
	    pip->progname,rs) ;

	goto badret ;

badret:
	ex = EX_DATAERR ;
	goto ret1 ;

}
/* end subroutine (main) */


/* local subroutines */



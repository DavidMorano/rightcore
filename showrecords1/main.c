/* main */

/* program to scan a tape for records */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0
#define	CF_DEBUG	0


/* revision history:

	= 1992-09-01, David Morano
	Program was originally written.

	= 1998-02-01, David Morano
	I revised the program to handle very large (greater than 1Gbyte)
	files without any fudging problems.  I should have done this
	from the very start !!  It was known in 1992 that files would
	one day get bigger !

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ showrecords [-maxnrec] [-r reclen] [infile]


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<cksum.h>
#include	<localmisc.h">

#include	"li.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	NPARG		1

#define	DEFRECLEN	((BLOCKSIZE * 126) * 10)
#define	MAXRECLEN	((BLOCKSIZE * 128) * 10)


/* external subroutines */

extern char	*strbasename() ;


/* external variables */


/* global varaiables */

struct global	g ;


/* local variables */


/* exported suroutines */


int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;
	CKSUM		sum ;
	LI		li_fbytes_p, li_fbytes_f, li_fbytes_t ;
	LI		li_tbytes_p, li_tbytes_f, li_tbytes_t ;
	LI		li_tmp ;
	unsigned long	fbytes_p = 0, fbytes_f = 0, fbytes_t = 0 ;
	unsigned long	tbytes_p = 0, tbytes_f = 0, tbytes_t = 0 ;
	unsigned long	fblocks_p = 0, fblocks_f = 0, fblocks_t = 0 ;
	unsigned long	tblocks_p = 0, tblocks_f = 0, tblocks_t = 0 ;
	unsigned long	mb, b ;
	uint		sv ;

	int	argr, argl, aol ;
	int	npa, i ;
	int	rs ;
	int	ifd ;
	int	f_debug = FALSE ;
	int	f_version = FALSE ;
	int	f_verbose = FALSE ;
	int	f_usage = FALSE ;
	int	maxnrec = MAXNREC ;
	int	nrec_full, nrec_part, nrec ;
	int	irec_full, irec_part, irec ;
	int	frec_p = 0, frec_f = 0, frec_t = 0 ;
	int	trec_p = 0, trec_f = 0, trec_t = 0 ;
	int	fmbytes, fbytes ;
	int	nfile = 0 ;
	int	len ;
	int	reclen = DEFRECLEN ;
	int	mf ;
	int	f_ignore = FALSE ;
	int	f_ignorezero = FALSE ;
	int	f_zero ;

	char	*argp, *aop ;
	char	*ifname = NULL ;
	char	ofname[MAXPATHLEN + 1] ;
	char	*blockbuf = NULL ;
	char	*reclenp = NULL ;


	g.progname = strbasename(argv[0]) ;

	if (bopen(efp,BFILE_STDERR,"wca",0666) >= 0)
		bcontrol(efp,BC_LINEBUF,0) ;


	g.debuglevel = 0 ;
	g.f.quiet = FALSE ;
	g.f.verbose = FALSE ;


	npa = 0 ;			/* number of positional so far */
	i = 1 ;
	argr = argc - 1 ;
	while (argr > 0) {

	    argp = argv[i++] ;
	    argr -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            if (isdigit(argp[1])) {

#if	CF_DEBUGS
	                debugprintf("main: dash_arg=%s\n",argp + 1) ;
#endif

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
	                        g.debuglevel = 1 ;
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
	                        g.f.quiet = TRUE ;
	                        break ;

	                    case 'v':
	                        f_verbose = TRUE ;
	                        break ;

	                    case 'z':
	                        f_ignorezero = TRUE ;
	                        break ;

	                    default:
	                        bprintf(efp,"%s: unknown option - %c\n",
	                            g.progname,*aop) ;

	                    case '?':
	                        f_usage = TRUE ;

	                    } ; /* end switch */

	                } /* end while */
	            }

	        } else {

	            npa += 1 ;	/* increment position count */

	        } /* end if */

	    } else {

	        if (npa < NPARG) {

	            switch (npa) {

	            case 0:
	                if (argl > 0) ifname = argp ;

	                break ;

	            default:
	                break ;
	            }

	            npa += 1 ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",
	                g.progname) ;

	        }

	    } /* end if */

	} /* end while (arguments) */


/* done w/ arguments, now handle miscellaneous */

#if	CF_DEBUG
	if (g.debuglevel > 1)
		debugprintf("main: finished parsing arguments\n") ;
#endif

	if (f_version) {

	    bprintf(efp,"%s: version %s\n",
	        g.progname,VERSION) ;

	    goto done ;
	}

	if (f_usage) goto usage ;


/* check arguments */

	if (reclenp == NULL) {

	    reclen = DEFRECLEN ;
	    if ((g.debuglevel > 0) || f_verbose) 
		bprintf(efp,
	        "%s: no record length given, using default %d\n",
	        g.progname,reclen) ;

	} else {

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

	    if (g.debuglevel > 0)
		bprintf(efp,
	        "%s: record string \"%s\"\n",
	        g.progname,reclenp) ;

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
	if (! g.f.quiet)
	    bprintf(efp,"%s: record length is too large - reduced to %d\n",
	        g.progname,reclen) ;

	}


	if ((rs = bopen(ofp,BFILE_STDOUT,"wct",0666)) < 0) 	
		goto badoutopen ;


	bprintf(ofp,"\n") ;

	bprintf(ofp,"record size for scanning:\t%d UNIX blocks\n",
	    (reclen / BLOCKSIZE)) ;

	if (g.debuglevel > 0) 
		bprintf(efp,"%s: running with record size %d\n",
	    g.progname,reclen) ;


/* allocate the buffer for the date from the tape */

	if ((blockbuf = valloc(reclen)) == NULL)
		goto badalloc ;


/* open files */

	if (ifname != NULL) {

	    if ((ifd = u_open(ifname,O_RDONLY,0666)) < 0)
	        goto badinfile ;

	} else 
	    ifd = 0 ;


/* finally go through the loops */

	if (g.debuglevel > 0)
	    bprintf(efp, "%s: about to enter while loop\n",
	        g.progname) ;

	f_zero = FALSE ;

	cksum_start(&sum) ;


/* enter here on ignorance */
ignore:
	fblocks_p = 0 ;
	fblocks_f = 0 ;
	fblocks_t = 0 ;
	fbytes_p = 0 ;
	fbytes_f = 0 ;
	fbytes_t = 0 ;
	frec_p = 0 ;
	frec_f = 0 ;
	frec_t = 0 ;

	fbytes = fmbytes = 0 ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
		debugprintf( "main: about to do 64 bits\n") ;
#endif

	li_zero(&li_fbytes_p) ;

	li_zero(&li_fbytes_f) ;

	li_zero(&li_fbytes_t) ;

	cksum_begin(&sum) ;

#if	CF_DEBUG
	if (debuglevel > 1)
		debugprintf( "main: entering loop\n") ;
#endif


	while (((len = u_read(ifd,blockbuf,reclen)) > 0) &&
	    (frec_t < maxnrec)) {

	    f_zero = FALSE ;
	    if (g.debuglevel > 0)
	        bprintf(efp,"%s: just read len=%d nrec=%d\n",
	            g.progname,len,nrec) ;

	    if (len == reclen) {

	        frec_f += 1 ;
	        fbytes_f += len ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
		debugprintf( "main: int64 (1)\n") ;
#endif

		li_load(&li_tmp,0,len) ;

		li_add2(&li_fbytes_f,&li_tmp) ;

	    } else {

	        frec_p += 1 ;
	        fbytes_p += len ;

#if	CF_DEBUG
	if (g.debuglevel > 1)
		debugprintf( "main: int64 (2)\n") ;
#endif

		li_load(&li_tmp,0,len) ;

		li_add2(&li_fbytes_p,&li_tmp) ;

	    }

		fbytes += len ;
		if (fbytes >= MEGABYTE) {

			fmbytes += (fbytes / MEGABYTE) ;
			fbytes = fbytes % MEGABYTE ;

		}


	    if ((len % BLOCKSIZE) != 0)
	        fblocks_p += 1 ;

	    fblocks_f += (len / BLOCKSIZE) ;

		cksum_accum(&sum,blockbuf,len) ;

	} /* end while */


#if	CF_DEBUG
	if (g.debuglevel > 1)
		debugprintf( "main: out of loop\n") ;
#endif

/* update this file's statistics */

	frec_t = frec_f + frec_p ;
	fblocks_t = fblocks_f + fblocks_p ;
	fbytes_t = fbytes_f + fbytes_p ;

	li_add3(&li_fbytes_t,&li_fbytes_f,&li_fbytes_p) ;

/* update total record statistics */

	trec_p += frec_p ;
	trec_f += frec_f ;
	trec_t = trec_f + trec_p ;

/* update total block statistics */

	tblocks_p += fblocks_p ;
	tblocks_f += fblocks_f ;
	tblocks_t = tblocks_f + tblocks_p ;

/* update total byte statistics */

	tbytes_p += fbytes_p ;
	tbytes_f += fbytes_f ;
	tbytes_t = tbytes_f + tbytes_p ;

	li_add2(&li_tbytes_p,&li_fbytes_p) ;

	li_add2(&li_tbytes_f,&li_fbytes_f) ;

	li_add3(&li_tbytes_t,&li_tbytes_f,&li_tbytes_p) ;

	cksum_end(&sum) ;

	cksum_getsum(&sum,&sv) ;



/* continue */

	if ((len == 0) && (! f_zero)) {

	    bprintf(ofp,"\n") ;

	    bprintf(ofp,
	        "file %d - EOF record at record # %d\n",
	        nfile,trec_t) ;

	    bprintf(ofp,"\trecords:\tP=%d F=%d T=%d\n",
	        frec_p,frec_f,frec_t) ;

	    bprintf(ofp,"\tUNIX blocks:\t%d%s\n",
	        fblocks_t,
		((fblocks_p) ? "+" : "")) ;

	    bprintf(ofp,"\tcksum:\t\t\\x%08x (%u)\n",sv,sv) ;

	    bprintf(ofp,"\tdata size:\t%d Mibytes and %d bytes\n",
		fmbytes,fbytes) ;


	    f_zero = TRUE ;
	    if (f_ignorezero) {

	        nfile += 1 ;
	        goto ignore ;
	    }

	} /* end if */

	rs = (trec_t > 0) ? 0 : 1 ;
	if (len < 0) {

	    bprintf(ofp,
	        "END-OF-MEDIA at record # %d (rs %d)\n",
	        trec_t,len) ;

	    if (f_ignore) {

	        nfile += 1 ;
	        goto ignore ;
	    }

	    rs = -1 ;

	} /* end if */


	cksum_gettotal(&sum,&sv) ;

	nfile += 1 ;
	bprintf(ofp,"\n") ;

	bprintf(ofp,"collective totals for all files seen (%d file%s) :\n",
	    nfile,(nfile == 1) ? "" : "s") ;

	bprintf(ofp,"\trecords:\tP=%ld F=%ld T=%ld\n",
	    trec_p,trec_f,trec_t) ;

	bprintf(ofp,"\tUNIX blocks:\t%ld%s\n",
	    tblocks_t,
		((tblocks_p) ? "+" : "")) ;

	bprintf(ofp,"\ttotal cksum:\t\\x%08x (%u)\n",
	    sv,sv) ;

	bprintf(ofp,"\tsize:\t\tapproximately %ld Mibytes\n",
	    (tblocks_t / 2048)) ;


	cksum_finish(&sum) ;

	bprintf(ofp,"\n") ;

/* finish off */

	u_close(ifd) ;

	bclose(ofp) ;

done:
	bclose(efp) ;

	return rs ;

badret:
	bclose(efp) ;

	return BAD ;

usage:
	bprintf(efp,
	    "%s: USAGE> %s [-?V] [-r reclen] [-z] [-nrec]",
	    g.progname,g.progname) ;

	bprintf(efp," [infile]\n") ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",
	    g.progname) ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument given (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;

badalloc:
	bprintf(efp,"%s: could not allocate buffer memory\n",
	    g.progname) ;

	goto badret ;

badinfile:
	bprintf(efp,"%s: cannot open the input file (rs %d)\n",
	    g.progname,ifd) ;

	goto badret ;

badoutopen:
	bprintf(efp,"%s: cannot open the output file (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;
}
/* end subroutine (main) */



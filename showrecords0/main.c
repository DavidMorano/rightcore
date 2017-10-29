/* main */

/* program to scan a tape for records */
/* last modified %G% version %I% */


#define		CF_DEBUGS	0
#define		CF_DEBUG	0


/* revision history:

	= Dave Morano, September 1992
	Program was originally written.


*/



/**************************************************************************

	Execute as :

		scanrec [-maxnrec] [-r reclen] [infile]


**************************************************************************/



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/unistd.h>
#include	<sys/param.h>
#include	<fcntl.h>
#include	<ctype.h>

#include	<bfile.h>

#include	"localmisc.h"
#include	"config.h"
#include	"defs.h"



/* local defines */

#define		NPARG		1

#define		DEFRECLEN	(BLOCKSIZE * 126)
#define		MAXRECLEN	(BLOCKSIZE * 128)



/* external subroutines */

extern int	bopen(), bclose() ;
extern int	bread(), bwrite() ;
extern int	bprintf() ;

extern char	*getenv() ;


/* external variables */

extern int	errno ;


/* global varaiables */

struct global	g ;


/* local variables */

static int	bpm = (1024 * 1024) / BLOCKSIZE ;




int main(argc,argv)
int	argc ;
char	*argv[] ;
{
	bfile		errfile, *efp = &errfile ;
	bfile		outfile, *ofp = &outfile ;

	unsigned long	fbytes_p = 0, fbytes_f = 0, fbytes_t = 0 ;
	unsigned long	fblocks_p = 0, fblocks_f = 0, fblocks_t = 0 ;
	unsigned long	tbytes_p = 0, tbytes_f = 0, tbytes_t = 0 ;
	unsigned long	tblocks_p = 0, tblocks_f = 0, tblocks_t = 0 ;
	unsigned long	mb, b ;

	int	argr, argl, aol ;
	int	npa, i ;
	int	rs, l ;
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
	int	nfile = 0 ;
	int	len ;
	int	reclen = DEFRECLEN ;
	int	mf ;
	int	f_ignore = FALSE ;
	int	f_ignorezero = FALSE ;
	int	f_zero ;

	char	*argp, *aop ;
	char	blockbuf[MAXRECLEN] ;
	char	*ifname = NULL ;
	char	ofname[MAXPATHLEN + 1] ;
	char	*reclenp = NULL ;


	g.progname = argv[0] ;
	bopen(efp,BERR,"wca",0666) ;

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

	                if (maxnrec < 0) maxnrec = MAXNREC ;

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
	                        f_version = TRUE ;
	                        break ;

	                    case 'v':
	                        f_verbose = TRUE ;
	                        break ;

/* record length */
	                    case 'r':
	                        if (argr <= 0) goto badargnum ;

	                        argp = argv[i++] ;
	                        argr -= 1 ;
	                        argl = strlen(argp) ;

	                        if (argl > 0) reclenp = argp ;

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

	if (f_debug) bprintf(efp,"finished parsing arguments\n") ;

	if (f_version) {

	    bprintf(efp,"%s version: %s\n",
	        g.progname,VERSION) ;

	    goto done ;
	}

	if (f_usage) goto usage ;

/* check arguments */

	if (reclenp == NULL) {

	    reclen = DEFRECLEN ;
	    if (f_debug || f_verbose) bprintf(efp,
	        "%s: no record length given, using default %d\n",
	        g.progname,reclen) ;

	} else {

	    l = strlen(reclenp) ;

	    mf = 1 ;
	    if (l > 0) {

	        if (reclenp[l - 1] == 'b') {

	            mf = BLOCKSIZE ;
	            reclenp[l-- - 1] = '\0' ;

	        } else if (reclenp[l - 1] == 'k') {

	            mf = 1024 ;
	            reclenp[l-- - 1] = '\0' ;

	        } else if (reclenp[l - 1] == 'm') {

	            mf = 1024 * 1024 ;
	            reclenp[l-- - 1] = '\0' ;

	        }

	    }

	    if (f_debug) bprintf(efp,
	        "%s: record string \"%s\"\n",
	        g.progname,reclenp) ;

	    if ((rs = cfdec(reclenp,l,&reclen)) < 0)
	        goto badarg ;

	    reclen = reclen * mf ;

	} /* end if */

	if (reclen > MAXRECLEN) {

	    reclen = MAXRECLEN ;
	    bprintf(efp,"%s: record length is too large - reduced to %d\n",
	        g.progname,reclen) ;

	}


	if ((rs = bopen(ofp,BFILE_STDOUT,"wct",0666)) < 0) 	
		goto badoutopen ;


	bprintf(ofp,"\n") ;

	bprintf(ofp,"record size for scanning:\t%d UNIX blocks\n",
	    reclen/512) ;

	if (f_debug) bprintf(efp,"%s: running with record size %d\n",
	    g.progname,reclen) ;

/* open files */

	if (ifname != NULL) {

	    if ((ifd = open(ifname,O_RDONLY,0666)) < 0)
	        goto badinfile ;

	} else 
	    ifd = 0 ;


/* finally go through the loops */

	if (f_debug) bprintf(efp,
	    "%s: about to enter while loop\n",
	    g.progname) ;

	f_zero = FALSE ;

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
	while (((len = read(ifd,blockbuf,MAXRECLEN)) > 0) &&
	    (frec_t < maxnrec)) {

	    f_zero = FALSE ;
	    if (f_debug)
	        bprintf(efp,"%s: just read len=%d nrec=%d\n",
	            g.progname,len,nrec) ;

	    if (len == reclen) {

	        frec_f += 1 ;
	        fbytes_f += len ;

	    } else {

	        frec_p += 1 ;
	        fbytes_p += len ;

	    }

	    if ((len % BLOCKSIZE) != 0)
	        fblocks_p += 1 ;

	    fblocks_f += (len / BLOCKSIZE) ;

	} /* end while */

/* update this file's statistics */

	frec_t = frec_f + frec_p ;
	fblocks_t = fblocks_f + fblocks_p ;
	fbytes_t = fbytes_f + fbytes_p ;

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

/* continue */

	if ((len == 0) && (! f_zero)) {

	    bprintf(ofp,"\n") ;

	    bprintf(ofp,
	        "file %d - EOF record at record # %d\n",
	        nfile,trec_t) ;

	    bprintf(ofp,"\trecords:\tP=%d F=%d T=%d\n",
	        frec_p,frec_f,frec_t) ;

	    bprintf(ofp,"\tUNIX blocks:\tP=%d F=%d T=%d, bytes %d\n",
	        fblocks_p,fblocks_f,fblocks_t,fbytes_t) ;

	    mb = fblocks_f / bpm ;
	    b = fbytes_t & ((1024 * 1024) - 1) ;
	    bprintf(ofp,"\tdata size:\t%d Mbytes and %d bytes\n",mb,b) ;

	    f_zero = TRUE ;
	    if (f_ignorezero) {

	        nfile += 1 ;
	        goto ignore ;
	    }

	} /* end if */

	rs = (trec_t > 0) ? 0 : 1 ;
	if (len < 0) {

	    bprintf(ofp,
	        "END-OF-MEDIA at record # %d (errno %d)\n",
	        trec_t,errno) ;

	    if (f_ignore) {

	        nfile += 1 ;
	        goto ignore ;
	    }

	    rs = -1 ;
	}

	nfile += 1 ;
	bprintf(ofp,"\n") ;

	bprintf(ofp,"collective totals for all files seen (%d file%s) :\n",
	    nfile,(nfile == 1) ? "" : "s") ;

	bprintf(ofp,"\trecords:\tP=%d F=%d T=%d\n",
	    trec_p,trec_f,trec_t) ;

	bprintf(ofp,"\tUNIX blocks:\tP=%d F=%d T=%d\n",
	    tblocks_p,tblocks_f,tblocks_t) ;

	mb = tblocks_f / bpm ;
	b = tbytes_t & ((1024 * 1024) - 1) ;
	bprintf(ofp,"\tdata size:\t%d Mbytes and %d bytes\n",mb,b) ;

	bprintf(ofp,"\n") ;

/* finish off */

	bclose(ofp) ;

	close(ifd) ;

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

badinfile:
	bprintf(efp,"%s: cannot open the input file (errno %d)\n",
	    g.progname,errno) ;

	goto badret ;

badoutopen:
	bprintf(efp,"%s: cannot open the output file (rs %d)\n",
	    g.progname,rs) ;

	goto badret ;
}
/* end subroutine (main) */




/* main */

/* parse out a Prairie report file and create a summary file */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1991-01-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1991 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This program will read the input file and reduce the data down to a
        brief summary report of the most important test case failure
        information.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<estrings.h>
#include	<localmisc.h>


/* local defines */

#define	DPRINT		0

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	2048
#endif

#define	TSIS_PRINT	0
#define	NFMOD		10
#define	NFDEF		20
#define	MAXDESC		20

#define	NPARG		2	/* number of positional arguments */

#define	TMPPREFIX	"/tmp/sum"
#define	NTRIES		12

#define	SS_FEATURE	"FEATURE TITLE"
#define	SS_PATH		"Now in"
#define	SS_FAILURE	"fail --"
#define	SS_SSTART	"SCRIPT BEGIN"
#define	SS_SEND		"SCRIPT END"
#define	SS_TSTART	"TSIS START"
#define	SS_TEND		"TSIS END"
#define	SS_BEGIN	"BEGIN"
#define	SS_END		"END"

/* define scan states */

#define	STATE_FINDNEXT	0
#define	STATE_SCANFAIL	2


/* external variables */


/* local variables */

char	separator[] =
"=======================================================================" ;


/* exported subroutines */


/* ARGSUSED */
int main(int argc,cchar **argv,cchar **envv)
{
	bfile		errfile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		infile, *ifp = &infile ;
	bfile		tmpfile, *tfp = &tmpfile ;

	long	tflen ;

	int	argl, aol ;
	int	rs ;
	int	len, mlen ;
	int	pan, i, j ;
	int	nilen, tslen ;
	int	nlines = NFDEF ;
	int	flen[NFMOD] ;
	int	pi ;			/* position index within buffer */
	int	fi ;			/* FIFO index */
	int	fn ;			/* FIFO number (# of FIFO entries) */
	int	ntotal = 0 ;
	int	nfail = 0 ;
	int	ndesc ;
	int	f_err, f_ni, f_feature ;
	int	f_desc = FALSE ;
	int	f_findpath, f_v = FALSE ;
	int	f_debug = FALSE ;
	int	f_ts, f_ss ;

	ushort	pid ;

	char	*argp, *aop ;
	char	*progname ;

	char	tmpfname[MAXPATHLEN + 1] ;
	char	linebuf[LINEBUFLEN], *lbp ;

	char	fbuf[NFMOD][LINEBUFLEN + 1] ;
	char	nibuf[LINEBUFLEN + 1] ;
	char	tsbuf[LINEBUFLEN + 1] ;

	char	*ifname = (char *) BFILE_STDIN ;
	char	*ofname = (char *) BFILE_STDOUT ;


	if (bopen(efp,BFILE_STDERR,"wca",0666) < 0) 
		return BAD ;

	progname = argv[0] ;

	rs = SR_OK ;

	pan = 0 ;			/* number of positional so far */
	i = 1 ;
	argc -= 1 ;

	while ((rs >= 0) && (argc > 0)) {

	    argp = argv[i++] ;
	    argc -= 1 ;
	    argl = strlen(argp) ;

	    if ((argl > 0) && (*argp == '-')) {

	        if (argl > 1) {

	            aop = argp ;
	            aol = argl ;

	            while (--aol) {

	                akp += 1 ;
	                switch ((int) *aop) {

	                case 'D':
	                    f_debug = TRUE ;
	                    break ;

	                case 'v':
	                    f_v = TRUE ;
	                    bprintf(efp,"%s version: %s\n",
	                        progname,VERSION) ;

	                    break ;

	                case 'n':
	                    if (argc <= 0) goto badnumarg ;

	                    argp = argv[i++] ;
	                    argc -= 1 ;
	                    argl = strlen(argp) ;

	                    if (cfdec(argp,argl,&nlines)) goto badarg ;

	                    if (nlines < 0) goto badarg ;

	                    break ;

	                case 'i':
	                    if (argc <= 0) goto badnumarg ;

	                    argp = argv[i++] ;
	                    argc -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) ifname = argp ;

	                    break ;

	                case 'o':
	                    if (argc <= 0) goto badnumarg ;

	                    argp = argv[i++] ;
	                    argc -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) ofname = argp ;

	                    break ;

	                default:
	                    bprintf(efp,"%s: unknown option - %c\n",
	                        progname,*aop) ;

	                    goto usage ;

	                } ; /* end switch */

	            } /* end while */

	        } else {

	            pan += 1 ;	/* increment position count */

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                if (argl > 0) ifname = argp ;

	                break ;

	            case 1:
	                if (argl > 0) ofname = argp ;

	                break ;

	            default:
	                break ;

	            }

	            pan += 1 ;

	        } else {

	            bprintf(efp,"%s: extra arguments ignored\n",progname) ;

	        }

	    } /* end if */

	} /* end while */

	if (f_debug) 
		bprintf(efp,"finished parsing arguments\n") ;

/* check arguments */

	if (nlines > NFMOD) 
		nlines = NFMOD ;

/* open files */

	rs = bopen(ifp,ifname,"r",0666) ;

	if (rs < 0) 
		goto badinfile ;

	rs = bopen(ofp,ofname,"wct",0666) ;

	if (rs < 0) 
		goto badoutopen ;

	if (f_debug) 
		bprintf(efp,"finished opening files\n") ;

/* perform initialization processing */

/* open a temporary file */

	pid = getpid() ;

	j = pid ;
	for (i = 0 ; i < NTRIES ; i += 1) {

	    sprintf(tmpfname,"%s%08X",TMPPREFIX,j + i) ;

	    if (bopen(tfp,tmpfname,"rwce",0664) >= 0) 
		break ;

	}

	if (i >= NTRIES) 
		goto badtmpfile ;

	u_unlink(tmpfname) ;

	if (f_debug) 
		bprintf(efp,"finished opening temp file \n") ;


/* find first bad script */
find_next:
	f_findpath = TRUE ;
	f_err = f_ni = FALSE ;
	f_feature = FALSE ;
	f_ts = f_ss = FALSE ;

	nilen = 0 ;

	len = 0 ;
	while (TRUE) {

	    lbp = linebuf ;
	    rs = breadline(ifp,lbp,LINEBUFLEN) ;

	    if (rs < 0)
		break ;

	    if (len == 0) 
		break ;

/* save all "now in"s until we get into the script */

	    if ((pi = substring(lbp,len,SS_PATH)) >= 0) {

	        nilen = len - pi - strlen(SS_PATH) ;
	        strncpy(nibuf,lbp + pi + strlen(SS_PATH),nilen) ;

	        nibuf[nilen] = '\0' ;
	        f_ni = TRUE ;
	        continue ;
	    }

/* scan for the start of a script */

	    if ((pi = substring(lbp,len,SS_TSTART)) >= 0) {
	        f_ts = TRUE ;
	        break ;
	    } else if ((pi = substring(lbp,len,SS_SSTART)) >= 0) {
	        f_ss = TRUE ;
	        break ;
	    }

	} /* end while */

	    if (rs < 0)
		goto badread ;

	    if (len == 0)
		goto finish ;

/* we found the start of a script, save the remainder of the start line */

	strncpy(tsbuf,lbp + pi + 6,len - pi - 6) ;

	tslen = len ;
	f_findpath = TRUE ;

/* increment the script count and rewind the temporary storage file */

	ntotal += 1 ;
	bseek(tfp,0L,0) ;

/* write out (to "temp" file) everything we've found up to this point */

	if (f_err)
	    bprintf(ofp,"\nerror in file format encountered\n") ;

	bprintf(ofp,"%s\n",separator) ;

#if	TSIS_PRINT
	bwrite(tfp,tsbuf,tslen) ;
#endif

#ifdef	COMMENT
	bputc(tfp,'\n') ;
#endif

/* print out the script path */

	if (f_ni) {
	    f_ni = FALSE ;
	    bwrite(tfp,nibuf,nilen) ;
	}

/* scan for the failure while searching for description lines */

	ndesc = 0 ;
	fi = 0 ;
	fn = 0 ;

#if	DPRINT
	bprintf(efp,"cleared the FIFO\n") ;
#endif

	len = 0 ;
	while (TRUE) {

	    lbp = fbuf[fi] ;
	    rs = breadline(ifp,lbp,LINEBUFLEN) ;

	    len = rs ;
	    if (rs < 0)
		break ;

	    if (len == 0)
		break ;

	    flen[fi] = len ;

/* continue scanning for PATH references */

	    if ((pi = substring(lbp,len,SS_PATH)) >= 0) {

	        nilen = len - pi - strlen(SS_PATH) ;
	        strncpy(nibuf,lbp + pi + strlen(SS_PATH),nilen) ;

	        nibuf[nilen] = '\0' ;
	        f_ni = TRUE ;
	        continue ;
	    }

/* end of handling a PATH reference */

/* scan for script feature title */

	    if (substring(lbp,len,SS_FEATURE) >= 0) {

	        if (f_ni) {
	            f_ni = FALSE ;
	            bwrite(tfp,nibuf,nilen) ;
	        }

	        f_findpath = FALSE ;
	        bwrite(tfp,lbp,len) ;

	        continue ;
	    }

/* end of handling the feature title */

/* scan for description while scanning for the failure */

	    if (len >= 8) {

	        pi = substring(lbp,len,SS_BEGIN) ;

	        if ((pi >= 0) && (pi <= 4)) {

	            if (f_ni) {
	                f_ni = FALSE ;
	                bwrite(tfp,nibuf,nilen) ;
	            }

	            f_desc = TRUE ;
	            bputc(tfp,'\n') ;

	            bwrite(tfp,lbp,len) ;

	            ndesc += 1 ;
	            f_findpath = FALSE ;
	            continue ;
	        }
	    }

/* end of finding & handling the description header */

/* continue to store away those lines inside of the description */

	    if (f_desc) {

	        if (ndesc < MAXDESC) {

	            bwrite(tfp,lbp,len) ;

	            ndesc += 1 ;
	            if (substring(lbp,len,SS_END) >= 0) {
	                f_desc = FALSE ;
	                bputc(tfp,'\n') ;
	            }

	        } else {

	            f_desc = FALSE ;
	            bputc(tfp,'\n') ;

	        }

	        continue ;
	    }

/* end of handling the description */

/* enter the current line into the "fail" line FIFO */

	    fi = (fi + 1) % NFMOD ;
	    fn += 1 ;

/* scan for a script failures */

	    if ((pi = substring(lbp,len,SS_FAILURE)) >= 0) {

#if	DPRINT
	        bprintf(efp,"** found the failure at FI %d\n",fi) ;
#endif

	        nfail += 1 ;

	        if (f_ni) {
	            bwrite(tfp,nibuf,nilen) ;
	        }

	        bputc(ofp,'\n') ;

/* write out all (but ONLY that much) that we have saved up until now */

/* length that we have written (remainder may be junk) */

	        tflen = bseek(tfp,0L,1) ;

	        bseek(tfp,0L,0) ;

/* copy just what we wrote to the temporary file */

	        while (tflen > 0) {

	            mlen = (LINEBUFLEN < tflen) ? LINEBUFLEN : tflen ;
	            rs = bread(tfp,linebuf,mlen) ;
		    len = rs ;
	            if (rs <= 0) break ;

	            tflen -= len ;
	            bwrite(ofp,linebuf,len) ;

	        } /* end while */

	            if (rs < 0) 
			goto badtread ;

/* write out the "fail line" FIFO */

#if	DPRINT
	        bprintf(efp,"** we have %d total fail lines\n",fn) ;

	        bprintf(efp,"** of which %d will be printed\n",nlines) ;
#endif

	        if (fn > nlines) 
			fn = nlines ;

	        j = (fi - fn + NFMOD) % NFMOD ;
	        for (i = 0 ; i < fn ; i += 1) {

#if	DPRINT
	            bprintf(efp,"writing buf # %d w/ len %d\n",j,flen[j]) ;
#endif

	            bwrite(ofp,fbuf + j,flen[j]) ;

#if	DPRINT
	            bwrite(efp,fbuf + j,flen[j]) ;
#endif

	            j = (j + 1) % NFMOD ;
	        }

#if	DPRINT
	        bprintf(efp,"done printing out FIFO\n") ;
#endif

	        break ;
	    }

/* end of handling script failure */

/* scan for "SCRIPT END" */

	    if (f_ts) {

	        if ((pi = substring(lbp,len,SS_TEND)) >= 0) 
			f_ts = FALSE ;

	    } else {

	        if ((pi = substring(lbp,len,SS_SEND)) >= 0) 
			f_ss = FALSE ;

	    }

	    if (! (f_ss || f_ts)) {

	        if (nilen > 0) {
	            bprintf(ofp,"\n%W",nibuf,nilen) ;
	        }

	        bprintf(ofp,"\nscript passed OK\n") ;

	        break ;
	    }

/* end of handling a premature "SCRIPT END" */

	} /* end while */

	if (rs < 0)
		goto badread ;

	if (len == 0)
		goto finish ;

	goto find_next ;

/* end of looping through the scripts */

/* we are done */
finish:
	bprintf(ofp,"%s\n",separator) ;

	bprintf(ofp,"\ntotal scripts scanned %d\n",ntotal) ;

	bprintf(ofp,"\ntotal scripts failed  %d\n",nfail) ;

	bprintf(ofp,"\n") ;

done:
	bclose(ifp) ;

	bclose(tfp) ;

	bclose(ofp) ;

badret:
	bclose(efp) ;

	return OK ;

usage:
	bprintf(efp,
	    "usage: %s [-v] [-n lines] [infile] [outfile]\n",
	    progname) ;

	goto badret ;

badnumarg:
	bprintf(efp,"%s: not enough arguments specified\n",progname) ;

	goto badret ;

badread:
	bprintf(efp,"%s: bad read on input file - %d\n",progname,len) ;

	goto	badret ;

badinfile:
	bprintf(efp,"%s: bad input file - %d\n",progname,rs) ;

	goto	badret ;

badoutopen:
	bprintf(efp,"%s: bad output file - %d\n",progname,rs) ;

	goto	badret ;

badtmpfile:
	bprintf(efp,"%s: could not open temporary file - errno %d\n",
	    progname,errno) ;

	goto	badret ;

badtread:
	bprintf(efp,
	    "%s: there was a bad read of the tmpfileoray file - %d\n",
	    progname,len) ;

	goto	badret ;

badarg:
	bprintf(efp,"%s: bad argument given\n",progname) ;

	goto badret ;
}

#ifdef	COMMENT

int scanc(buf,n,c)
int	n, c ;
char	*buf ;
{
	int	j ;

	for (j = 0 ; j < n ; j += 1) {

	    if (buf[j] == c) 
		return j ;

	} /* end for */

	return -1 ;
}

#endif /* COMMENT */



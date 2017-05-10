/* sat_fe */

/* last modified %G% version %I% */
/* Sat Filter Compile program */


#define	CF_DEBUGS	0
#define	CF_AUDIT	1


/* revision history:

	= 1991-11-01, David A­D­ Morano

	- total rewrite from scratch


*/

/* Copyright © 1991 David A­D­ Morano.  All rights reserved. */

/****************************************************************************

 * This program converts extension and circuit pack locations in SATSIM     
 * scripts (.ap files) to model specific locations at script run time.
 * It reads the model file (dr06_CP.s for instance) to 
 * determine what extension perfix and carrier-slot location to 
 * replace in the file.

	Synopsis:

	$sat_fc [-f filter_file] [input_file]


****************************************************************************/


#Include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<estrings.h>
#include	<field.h>
#include	<localmisc.h>

#include	"sat_f.h"


/* local defines */

#define	VERSION		"0"
#define	NPARG		2
#define	KEYLEN		PATHLEN
#define	NULL		((char *) 0)


/* external subroutines */


/* forward references */

int	getconfig(), ourfreeze() ;
int	basename(), fullpath() ;

mkname() ;


/* local structures */

struct gdata {
	char	*progname ;
	bfile	*efp ;
	int	f_debug ;
} ;


/* exported subroutines */


int main(argc,argv,envv)
int	argc ;
char	*argv[] ;
char	*envv[] ;
{
	bfile	errfile, *efp = &errfile ;
	bfile	outfile, *ofp = &outfile ;
	bfile	infile, *ifp = &infile ;

	struct fzentry	f[NFILTERS] ;

#ifdef	COMMENT
	struct ustat	statbuf ;
#endif

	struct gdata	g, *gdp = &g ;

	int	argl, aol ;
	int	pan, i = 0 ;
	int	nfe, fbuflen ;
	int	rs ;
	int	len, lenr, l ;
	int	pi ;
	int	keylen ;
	int	f_usage = FALSE ;
	int	f_badnokey = FALSE ;

	char	*argp, *aop ;
	char	*progname ;
#ifdef	COMMENT
	char	fzfname[PATHLEN] ;
#endif
	char	fbuf[FBUFLEN] ;
	char	*infname = NULL ;
	char	*outfname = NULL ;
	char    *ffname = NULL ;
#ifdef	COMMENT
	char	*bnp ;
#endif
	char	linebuf[LINELEN], *lbp ;
#ifdef	COMMENT
	char	keybuf[KEYLEN + 1] ;
#endif
	char	buf[LINELEN], *bp ;


	if ((rs = bopen(efp,BERR,"wca",0666)) < 0) return rs ;

/* check compile time assertions */

	if (PATHLEN < (strlen(FZPREFIX) + 16)) {

	    bprintf(efp,"%s: compiled freeze path prefix is too long\n",
	        progname) ;

	    rs = PR_BAD ;
	    goto badret ;
	}

/* continue with initialization */

	gdp->efp = efp ;
	gdp->progname = argv[0] ;
	progname = argv[0] ;
	gdp->f_debug = FALSE ;

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

	                case 'f':
	                    if (argc <= 0) goto badargnum ;

	                    argp = argv[i++] ;
	                    argc -= 1 ;
	                    argl = strlen(argp) ;

	                    ffname = argp ;
	                    break ;

	                case 'D':
	                    gdp->f_debug = TRUE ;
	                    break ;

	                case 'V':
	                    bprintf(efp,"%s: SAT Filter Compile - ",
	                        progname) ;

	                    bprintf(efp,"version %s\n",
	                        VERSION) ;

	                    break ;

	                default:
	                    bprintf(efp,"%s: unknown option - %c\n",
	                        progname,*aop) ;
	                    rs = PR_UNKNOWN ;

	                case '?':
	                    f_usage = TRUE ;

	                } /* end switch */

	            }

	        } else {

	            pan += 1 ;	/* increment position count */

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                if (argl > 0) infname = argp ;

	                break ;

	            case 1:
	                if (argl > 0) outfname = argp ;

	                break ;

	            default:
	                break ;
	            }

	            pan += 1 ;

	        } else {

	            bprintf(efp,
	                "%s: extra arguments ignored\n",progname) ;

	        }

	    } /* end if */

	}  /* end while */

	if (f_usage) goto usage ;

/* check the arguments */

	if (ffname == NULL) ffname = getenv("A_FILTER") ;

	if (ffname == NULL) ffname = getenv("CPLOC") ;

	if (ffname == NULL) {

	    bprintf(efp,
	        "%s: no filter file is specified\n",progname) ;

	    rs = PR_BAD ;
	    goto badret ;
	}

	if (gdp->f_debug)
	    bprintf(efp,"%s: using filter file \"%s\"\n",
	        progname,ffname) ;

	if ((rs = access(ffname,04)) < 0) goto badfilter ;


/* check the freeze file */

	rs = getconfig(gdp,ffname,f,fbuf,&nfe,&fbuflen) ;

	switch (rs) {

	case GC_BADPATH:
	    bprintf(gdp->efp,"%s: no full path of filter file\n",
	        gdp->progname) ;

	    break ;

	case GC_TOOMANY:
	    bprintf(gdp->efp,"%s: too many filter entries\n",
	        gdp->progname) ;

	    break ;

	case GC_TOOMUCH:
	    bprintf(gdp->efp,"%s: ran out of string space\n",
	        gdp->progname) ;

	    break ;

	case GC_MAXTMP:
	    bprintf(gdp->efp,"%s: maximum temporary file attempts\n",
	        gdp->progname) ;

	    break ;

	default:
	case OK:
	    ;
	}

	if (rs < 0) goto badfreeze ;

/* open the files to operate on */

	if (infname == NULL) infname = ((char *) BIN) ;

	if ((rs = bopen(ifp,infname,"r",0666)) < 0) goto badinfile ;


	if (outfname == NULL) outfname = ((char *) BOUT) ;

	if ((rs = bopen(ofp,outfname,"wct",0666)) < 0) goto badoutfile ;

/* do it */

	while ((len = breadline(ifp,linebuf,LINELEN)) > 0) {

#if	CF_DEBUGS
	    bprintf(efp,"got a line\n") ;
#endif

	    lenr = len ;
	    lbp = linebuf ;
	    while ((pi = substring(lbp,lenr,"$\173")) >= 0) {

#if	CF_DEBUGS
	        bprintf(efp,"got a sub pattern at offset %d\n",pi) ;
#endif

/* pretend that the line is smaller now */

	        lenr -= pi ;
	        lbp += pi ;

/* we have a substitution key, read out the key string */

	        keylen = substring(lbp + 2,lenr - 2,"\175") ;

	        if (keylen < 0) goto badinput ;

#if	CF_DEBUGS
	        bprintf(efp,"keylen %d\n",keylen) ;
#endif

	        (lbp + 2)[keylen] = '\0' ;

#if	CF_DEBUGS
	        bprintf(efp,"trying	filter		key\n") ;
#endif

	        for (i = 0 ; i < nfe ; i += 1) {

#if	CF_DEBUGS
	            bprintf(efp,"	%W		%W\n",
	                f[i].kp,f[i].klen,lbp + 2,keylen) ;
#endif

	            if ((f[i].klen == keylen) && 
	                (strcmp(lbp + 2,f[i].kp) == 0)) break ;

	        }

	        if (i >= nfe) {

	            f_badnokey = TRUE ;
	            bprintf(efp,
	                "%s: unknown key string encountered\n",
	                progname) ;

	            bprintf(efp,
	                "	%W\n",lbp + 2,keylen) ;

/* substitute the name alone in place of the key pattern introduction */

	            bp = buf ;
	            movc(2,"**",bp) ;

	            bp += 2 ;
	            movc(keylen,lbp + 2,bp) ;

	            bp += keylen ;
	            movc(2,"**",bp) ;

	            bp += 2 ;

/* copy over the remainder of the original line */

	            l = lenr - keylen - 3 ;
	            movc(l,lbp + keylen + 3,buf + keylen + 4) ;

/* copy the modified portion of the line back to the 'linebuf' buffer */

	            lenr = lenr + 1 ;
	            movc(lenr,buf,lbp) ;

	            len = len + 1 ;

	        } else {

/* we got a key match, substitute for it in an alternate buffer */

	            if ((LINELEN - lenr) < f[i].slen) goto badsub ;

/* substitute the alternate string in place of the key pattern */

	            movc(f[i].slen,f[i].sp,buf) ;

/* copy over the remainder of the original line */

	            l = lenr - keylen - 3 ;
	            movc(l,lbp + keylen + 3,buf + f[i].slen) ;

/* copy the modified portion of the line back to the 'linebuf' buffer */

	            lenr = lenr - keylen - 3 + f[i].slen ;
	            movc(lenr,buf,lbp) ;

	            len = len - keylen - 3 + f[i].slen ;
	        }
	    }

/* wrtite the (possibly modified) line to the output */

	    bwrite(ofp,linebuf,len) ;

	}

	if (f_badnokey) rs = PR_BADNOKEY ;

	else rs = PR_OK ;

/* done */

	bclose(ifp) ;

	bclose(ofp) ;

	bclose(efp) ;

	return rs ;

badargnum:
	bprintf(efp,"%s: not enough arguments given\n",progname) ;

	rs = PR_BAD ;
	goto badret ;

#ifdef	COMMENT
badparam:
	bprintf(efp,"%s: bad parameter specified\n",progname) ;

	rs = PR_BAD ;
	goto badret ;
#endif

usage:
	bprintf(efp,
	    "usage: %s [-f filter] [input] [outfile] [-D] [-V] [-?]\n",
	    progname) ;

	rs = PR_OK ;

badret:
	bclose(efp) ;

	return rs ;

badfilter:
	rs = PR_BADFILTER ;
	goto badret ;

badfreeze:
	bprintf(efp,"%s: problem with the SAT filter freeze file (%d)\n",
	    progname,rs) ;

	rs = PR_BAD ;
	goto badret ;

badinfile:
	bprintf(efp,"%s: can not open the input file (%d)\n",
	    progname,rs) ;

	rs = PR_BAD ;
	goto badret ;

badoutfile:
	bprintf(efp,"%s: can not open the output file (%d)\n",
	    progname,rs) ;

	bclose(ifp) ;

	rs = PR_BAD ;
	goto badret ;

badsub:
	bprintf(efp,"%s: substitution string is too long - %d\n",
	    progname,f[i].slen) ;

	rs = PR_BAD ;
	goto badcleanup ;

badinput:
	bprintf(efp,"%s: bad input key substitution format\n",
	    progname) ;

	rs = PR_BAD ;

badcleanup:
	bclose(ifp) ;

	bclose(ofp) ;

	goto badret ;
}


/* check to see if we have a good freeze file */

/*
	The freeze file has the following layout :

	magic number
	number filter entries
	filter buffer pointer
	filter buffer length
	filter entry array block
	filter buffer

*/


int getconfig(gdp,ffname,fe,fbuf,nfep,fbuflenp)
struct gdata	*gdp ;
char		ffname[] ;
struct fzentry	fe[] ;
char		fbuf[] ;
int		*nfep ;
int		*fbuflenp ;
{
	bfile	filterfile, *ffp = &filterfile ;
	bfile	fzfile, *zfp = &fzfile ;

	struct ustat	fzstat ;
	struct ustat	fstat ;

	FIELD	fsb ;

	long	fzmagic, num1, num2 ;

	int	line, len, bnlen ;
	int	nfe = 0 ;
	int	fbuflen = 0, rflen ;
	int	rs, i, c ;

	char	fzfname[PATHLEN] ;
	char	pathbuf[PATHLEN] ;
	char	*bn ;
	char	*cp, *fbp = fbuf ;
	char	linebuf[LINELEN] ;


	if (stat(ffname,&fstat) < 0) return BAD ;

	if (fullpath(ffname,pathbuf,PATHLEN) < 0) goto badpath ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"len of pathbuf %d\n",strlen(pathbuf)) ;
#endif

/* create the freeze file name */

	for (i = 0 ; i < MAXTMP ; i += 1) {

	    mkname(ffname,i++,fzfname) ;

	    if ((rs = ourfreeze(gdp,zfp,fzfname,pathbuf)) >= 0) break ;

	}

	if (i >= MAXTMP) return GC_MAXTMP ;

	if (gdp->f_debug)
	    bprintf(gdp->efp,"%s: freeze file name \"%s\"\n",
	        gdp->progname,fzfname) ;

/* if file doesn't exist, go create it */

#if	CF_DEBUGS
	bprintf(gdp->efp,"1\n") ;
#endif

	if (rs > 0) goto create ;

/* check file modification dates */

#if	CF_DEBUGS
	bprintf(gdp->efp,"2\n") ;
#endif

	if (stat(fzfname,&fzstat) < 0) goto badfreeze ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"3\n") ;
#endif

	if (fstat.st_mtime > fzstat.st_mtime) goto badfreeze ;

/* read in and check the data structure addresses and sizes */

#if	CF_DEBUGS
	bprintf(gdp->efp,"4\n") ;
#endif

	if (bread(zfp,&nfe,sizeof(int)) < sizeof(int)) goto badfreeze ;

	*nfep = nfe ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"nfe %d\n",nfe) ;
#endif

#if	CF_DEBUGS
	bprintf(gdp->efp,"5\n") ;
#endif

	if (nfe < 0) goto badfreeze ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"6\n") ;
#endif

	if (bread(zfp,&fbp,sizeof(fbp)) < sizeof(fbp)) goto badfreeze ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"7\n") ;
#endif

	if (fbp != fbuf) goto badfreeze ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"8\n") ;
#endif

	if (bread(zfp,&fbuflen,sizeof(int)) < sizeof(int)) goto badfreeze ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"9\n") ;
#endif

	if (nfe > NFILTERS) {

	    bclose(zfp) ;

	    return GC_TOOMANY ;
	}

#if	CF_DEBUGS
	bprintf(gdp->efp,"10\n") ;
#endif

	*fbuflenp = fbuflen ;
	if (fbuflen > FBUFLEN) {

	    bclose(zfp) ;

	    return GC_TOOMUCH ;
	}

#if	CF_DEBUGS
	bprintf(gdp->efp,"11\n") ;
#endif

	if (fbuflen < 0) goto badfreeze ;

	len = sizeof(struct fzentry) * nfe ;

/* read in the freeze file data */

#if	CF_DEBUGS
	bprintf(gdp->efp,"12\n") ;
#endif

	if (bread(zfp,fe,len) < len) goto badfreeze ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"13\n") ;
#endif

	if (bread(zfp,fbuf,fbuflen) < fbuflen) goto badfreeze ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"14\n") ;
#endif

	if (gdp->f_debug) {

	    bprintf(gdp->efp,"%s: filter substitutions\n",
	        gdp->progname) ;

	    for (i = 0 ; i < nfe ; i += 1) {

	        bprintf(gdp->efp,"%W\n-> %W\n",fe[i].kp,fe[i].klen,
	            fe[i].sp,fe[i].slen) ;

	    }
	}

	bclose(zfp) ;

	return OK ;

/* process the filter file and write out the freeze file */
create:
	if (gdp->f_debug)
	    bprintf(gdp->efp,"%s: creating a new freeze file\n",
	        gdp->progname) ;

/* process the filter file first */

	if (bopen(ffp,ffname,"r") < 0) return BAD ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"open successful\n") ;
#endif

	nfe = 0 ;
	rflen = FBUFLEN ;
	fbp = fbuf ;
	line = 0 ;
	while ((len = breadline(ffp,linebuf,LINELEN)) > 0) {

#if	CF_DEBUGS
	    bprintf(gdp->efp,"read filter file line\n") ;
#endif

	    line += 1 ;
	    if (len == 1) continue ;	/* blank line */

	    if (linebuf[len - 1] != '\n') {

	        while ((c = bgetc(ffp)) >= 0) if (c == '\n') break ;

	        bprintf(gdp->efp,
	            "%s: filter file line too long, ignoring\n",
	            gdp->progname) ;

	        continue ;
	    }

	    fsb.rlen = len - 1 ;
	    fsb.lp = linebuf ;
	    cp = fbp ;

/* parse out the key string */

	    field(&fsb,0L) ;

	    if (fsb.flen == 0) continue ;

	    if (nfe >= NFILTERS) {

	        bclose(ffp) ;

	        return GC_TOOMANY ;
	    }

	    if ((fsb.flen + 1) >= rflen) goto toomuch ;

	    fe[nfe].kp = cp ;
	    fe[nfe].klen = fsb.flen ;
	    strncpy(cp,fsb.fp,fsb.flen) ;

	    cp += fsb.flen ;
	    *cp++ = '\0' ;
	    rflen -= (fsb.flen + 1) ;

/* try to parse out the substitution string */

	    field(&fsb,0L) ;

	    if (fsb.flen == 0) {

	        bprintf(gdp->efp,"%s: error in filter file on line %d\n",
	            gdp->progname,line) ;

	        continue ;
	    }

	    if ((fsb.flen + 1) >= rflen) goto toomuch ;

	    fe[nfe].sp = cp ;
	    fe[nfe].slen = fsb.flen ;
	    strncpy(cp,fsb.fp,fsb.flen) ;

	    cp += fsb.flen ;
	    *cp++ = '\0' ;
	    rflen -= (fsb.flen + 1) ;
	    fbp = cp ;
	    nfe += 1 ;

	} /* end while */

	fbuflen = fbp - fbuf ;

#if	CF_AUDIT
	if (fbuflen != (FBUFLEN - rflen)) {

	    bprintf(gdp->efp,
	        "%s: audit failure on \"filter buffer length\"\n",
	        gdp->progname) ;

	}
#endif

#if	CF_DEBUGS
	fbp = fbuf ;
	for (i = 0 ; i < nfe ; i += 1) {

	    bprintf(gdp->efp,"%s\n",fbp) ;

	    fbp += (strlen(fbp) + 1) ;

	    bprintf(gdp->efp,"-> %s\n",fbp) ;

	    fbp += (strlen(fbp) + 1) ;

	}
#endif

#if	CF_DEBUGS
	bprintf(gdp->efp,"read whole filter file\n") ;
#endif

	bclose(ffp) ;

	*nfep = nfe ;
	*fbuflenp = fbuflen ;

/* try to write out a freeze file */

	unlink(fzfname) ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"opening freeze file\n") ;
#endif

	if ((rs = bopen(zfp,fzfname,"rwct",0666)) < 0) {

	    bprintf(gdp->efp,"%s: could not create a freeze file (%d)\n",
	        gdp->progname,rs) ;

	    return OK ;
	}

#if	CF_DEBUGS
	bprintf(gdp->efp,"writing out magic\n") ;
#endif

	fzmagic = FZMAGIC ;
	bwrite(zfp,&fzmagic,sizeof(fzmagic)) ;

	bprintf(zfp,"%s\n",pathbuf) ;

/* write out the freeze file data */

#if	CF_DEBUGS
	bprintf(gdp->efp,"writing out freeze file data\n") ;
#endif

	bwrite(zfp,&nfe,sizeof(int)) ;

	fbp = fbuf ;
	bwrite(zfp,&fbp,sizeof(fbp)) ;

	bwrite(zfp,&fbuflen,sizeof(int)) ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"filter buffer length %d\n",fbuflen) ;
#endif

	bwrite(zfp,fe,sizeof(struct fzentry) * nfe) ;

	bwrite(zfp,fbuf,fbuflen) ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"wrote out freeze file data\n") ;
#endif

done:
	bclose(zfp) ;

	return OK ;

badpath:
	if (gdp->f_debug)
	    bprintf(gdp->efp,
	        "%s: could not get full path of filter file\n",
	        gdp->progname) ;

	return GC_BADPATH ;

badfreeze:
	if (gdp->f_debug)
	    bprintf(gdp->efp,"%s: bad freeze file\n",
	        gdp->progname) ;

	bclose(zfp) ;

	goto create ;

toomuch:
	bclose(ffp) ;

	return GC_TOOMUCH ;
}


int fullpath(filename,pathbuf,pathlen)
char	filename[] ;
char	pathbuf[] ;
int	pathlen ;
{
	int	plen ;
	int	flen = strlen(filename) ;


	if (filename[0] == '/') {

	    if ((flen + 1) >= pathlen) return BAD ;

	    strcpy(pathbuf,filename) ;

	    return OK ;
	}

	if (getpwd(pathbuf,pathlen) < 0) return BAD ;

	plen = strlen(pathbuf) ;

	if ((pathlen - plen) < (flen + 2)) return BAD ;

	strcpy(pathbuf + plen,"/") ;

	strcpy(pathbuf + plen + 1,filename) ;

	return OK ;
}


mkname(ffname,seed,fzfname)
char	*ffname ;
int	seed ;
char	fzfname[] ;
{
	long	num1, num2 ;

	int	len, bnlen, i ;

	char	*bn ;


	if (! basename(ffname,strlen(ffname),&bn,&bnlen)) {

	    bn = "junk" ;
	    bnlen = strlen(bn) ;

	}

	num1 = 0 ;
	for (i = 0 ; i < ((bnlen < 6) ? bnlen : 6) ; i += 1) {

	    num1 = (num1 << 5) | (bn[i] & 0x1F) ;

	}

	num2 = 0 ;
	if (bnlen > 6) {

	    for (i = 6 ; i < ((bnlen < 12) ? bnlen : 12) ; i += 1) {

	        num2 = (num2 << 5) | (bn[i] & 0x1F) ;

	    }

	    num1 = num1 ^ (num2 >> 16) ;

	    switch (bnlen) {

	    case 14:
	        num2 = num2 ^ bn[13] ;

	    case 13:
	        num1 = num1 ^ bn[12] ;
	    }
	}

	num1 = num1 ^ seed ;
	len = sprintf(fzfname,"%s/%s%08X%04X",
	    FZPATH,FZPREFIX,num1,num2) ;

	fzfname[len] = '\0' ;
}


int ourfreeze(gdp,zfp,fzfname,pathbuf)
struct gdata	*gdp ;
bfile		*zfp ;
char		fzfname[], pathbuf[] ;
{
	long	fzmagic ;

	int	i, len, rs ;

	char	buf[PATHLEN], *bp, *cp ;


	rs = bopen(zfp,fzfname,"r",0666) ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"entered 'ourfreeze' - open RS (%d)\n",rs) ;
#endif

	if (rs < 0) return 1 ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"1\n") ;
#endif

	if (bread(zfp,&fzmagic,sizeof(fzmagic)) < sizeof(fzmagic))
	    goto bad ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"2\n") ;
#endif

	if (fzmagic != FZMAGIC) goto bad ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"3\n") ;
#endif

	if ((len = breadline(zfp,buf,PATHLEN)) < 0) goto bad ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"4\n") ;
#endif

	if (len < 2) goto bad ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"5\n") ;
#endif

	if (buf[len - 1] != '\n') goto bad ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"6\n") ;
#endif

	len -= 1 ;
	bp = buf ;
	for (i = 0 ; i < len ; i += 1) {

#if	CF_DEBUGS
	    bprintf(gdp->efp,"c=%c %02X\n",*bp,*bp) ;
#endif

	    if ((*bp != ' ') && (*bp != '\t')) break ;

	    bp += 1 ;
	}

	if (i >= len) goto bad ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"7\n") ;
#endif

	cp = bp ;
	for ( ; i < len ; i += 1) {

	    if ((*cp == ' ') || (*cp == '\t')) break ;

	    cp += 1 ;
	}

#if	CF_DEBUGS
	bprintf(gdp->efp,"8\n") ;
#endif

	*cp = '\0' ;
	if (strcmp(bp,pathbuf) != 0) goto bad ;

#if	CF_DEBUGS
	bprintf(gdp->efp,"9\n") ;
#endif

	return OK ;

bad:
	bclose(zfp) ;

	return BAD ;
}


int basename(s,sl,bp,blp)
char	*s ;
int	sl ;
char	**bp ;
int	*blp ;
{
	int	si ;

	for (si = sl ; si > 0 ; si -= 1) {

	    if (s[si - 1] == '/') break ;

	}

	*bp = s + si ;
	*blp = sl - si ;
	return (*blp) ;
}



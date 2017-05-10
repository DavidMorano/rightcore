/* main */

/* insert bibliographical references into a document formated with MM macros */
/* last modified %G% version %I% */


#define	CF_DEBUG	0		/* run-time debugging */


/*
	= 1992-03-01, David A.D. Morano
	
*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Synopsis:

	$ updatedir [<infile>] | ... | troff -mm


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<ctype.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"


/* local defines */

#define		NPARG		1


/* external subroutines */


/* external variables */


/* forward subroutine references */

void		init_bufdesc(), init_br() ;


/* local structure definitions */

struct bufdesc {
	int	len ;
	char	buf[BIBLEN] ;
} ;

struct br {
	struct bufdesc	a ;
	struct bufdesc	t ;
	struct bufdesc	d ;
	struct bufdesc	j ;
	struct bufdesc	m ;
	struct bufdesc	i ;
	struct bufdesc	p ;
	struct bufdesc	c ;
} ;



int main(argc,argv,envv)
int		argc ;
const char	*argv[] ;
const char	*envv[] ;
{
	struct br	entry ;
	bfile		errfile, *efp = &errfile ;
	bfile		infile, *ifp = &infile ;
	bfile		outfile, *ofp = &outfile ;
	bfile		lookifile, *lifp = &lookifile ;
	bfile		lookofile, *lofp = &lookofile ;
	bfile		lookefile, *lefp = &lookefile ;
	bfile		*fpa[3] ;
	pid_t		pid_lookbib ;

	int		argr, argl, aol ;
	int		pan = 0 ;
	int		i ;
	int		l, len, rs ;
	int		macrolen ;
	int		macrolen1 = strlen(MACRONAME1) ;
	int		macrolen2 = strlen(MACRONAME2) ;
	int		bo ;
	int		rlen ;
	int		ii ;
	int	f_usage = FALSE ;
	int	f_debug = FALSE ;
	int	f_version = FALSE ;
	int	f_dash = FALSE ;
	int	f_verbose = FALSE ;
	int	f_ignore = FALSE ;
	int	f_begin = TRUE ;
	int	f_end ;
	int	f_pass ;
	int	f_lookopen = FALSE ;
	int	f_checkangle, f_sawangle, f_done, f_record ;
	int	f_double ;
	int	f_bibeol ;
	int	f_needquoteend ;
	int	f_started ;

	char	*argp, *aop ;
	char	*progname ;
	char	*database = DEFDATABASE ;
	char	linebuf[LINELEN + 1] ;
	char	bibbuf[BIBLEN + 1] ;
	char	cmd_lookbib[(MAXPATHLEN * 2) + 1] ;
	char	*infname = NULL ;
	char	*cp, *rbuf ;
	char	c, c1, c2 ;


	progname = argv[0] ;
	if (bopen(efp,BERR,"wca",0666) < 0) return BAD ;

	bcontrol(efp,BC_LINEBUF) ;

/* initial stuff */

	if ((cp = getenv("BIBLIOGRAPHY")) != NULL) database = cp ;


/* go to the arguments */

	i = 1 ;
	argr = argc - 1 ;
	while (argr > 0) {

	    argp = argv[i++] ;
	    argr -= 1 ;
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

	                case 'V':
	                    bprintf(efp,"%s: version %s\n",
	                        progname,VERSION) ;

				f_version = TRUE ;
	                    break ;

	                case 'v':
	                    f_verbose = TRUE ;
	                    break ;

	                case 'p':
	                    if (argr <= 0) goto badargnum ;

	                    argp = argv[i++] ;
	                    argr -= 1 ;
	                    argl = strlen(argp) ;

	                    if (argl) database = argp ;

	                    break ;

	                case 'i':
	                    f_ignore = TRUE ;
	                    break ;

	                default:
	                    bprintf(efp,"%s: unknown option - %c\n",
	                        progname,*aop) ;

	                case '?':
	                    f_usage = TRUE ;

	                } /* end switch */

	            } /* end while */

	        } else {

	            pan += 1 ;	/* increment position count */
	            f_dash = TRUE ;

	        } /* end if */

	    } else {

	        if (pan < NPARG) {

	            switch (pan) {

	            case 0:
	                if (argl > 0) infname = argp ;

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
	    bprintf(efp,"finished parsing arguments\n") ;

	if (f_usage) goto usage ;

	if (f_version) goto done ;

/* check arguments */

	sprintf(linebuf,"%s.ia",database) ;

	if ((access(database,R_OK) < 0) &&
		(access(linebuf,R_OK) < 0)) goto baddatabase ;

/* open files */

	if (infname == NULL) infname = (char *) 0 ;

	if ((rs = bopen(ifp,infname,"r",0666)) < 0)
	    goto badinfile ;

	if ((rs = bopen(ofp,(char *) 1,"wct",0666)) < 0)
	    goto badoutopen ;

/* other initializations */

	fpa[0] = lifp ;
	fpa[1] = lofp ;
	fpa[2] = NULL ;

/* go through the loops */

	f_begin = TRUE ;
	while ((len = breadline(ifp,linebuf,LINELEN)) > 0) {

	    f_end = TRUE ;
	    if (linebuf[len - 1] != '\n') f_end = FALSE ;

#if	DEBUG
	    if (f_debug)
	        bprintf(efp,"processing a line begin=%d end=%d\n%W",
	            f_begin,f_end,linebuf,len) ;
#endif

	    macrolen = 0 ;
	    if (f_begin) {

	        if ((len >= (macrolen1 + 1)) &&
	            (strncmp(linebuf,MACRONAME1,macrolen1) == 0))
	            macrolen = macrolen1 ;

	        if ((! macrolen) && (len >= (macrolen2 + 1)) &&
	            (strncmp(linebuf,MACRONAME2,macrolen2) == 0))
	            macrolen = macrolen2 ;

	    }

	    if (macrolen) {

#if	DEBUG
	debugprintf("input line >\n%W",
		linebuf,len) ;

	for (ii = 0 ; ii < len ; ii += 1)
		debugprintf(" %02X",linebuf[ii]) ;

	debugprintf("\n") ;
#endif

	        if (len > (macrolen + 1)) {

	            if (f_debug)
	                bprintf(efp,"got a bibliographical reference\n") ;

#if	DEBUG
	for (ii = 0 ; ii < len ; ii += 1)
		debugprintf(" %02X",linebuf[ii]) ;

	debugprintf("\n") ;
#endif

	            if (! f_lookopen) {

#if	DEBUG
	for (ii = 0 ; ii < len ; ii += 1)
		debugprintf(" %02X",linebuf[ii]) ;

	debugprintf("\n") ;

	debugprintf("database %d > %s\n",strlen(database),database) ;

	for (ii = 0 ; ii < strlen(database) ; ii += 1)
		debugprintf(" %02X",database[ii]) ;

	for (ii = 0 ; ii < (MAXPATHLEN*2) ; ii += 1) 
		cmd_lookbib[ii] = 0x55 ;

	debugprintf("\n") ;
#endif
	                f_lookopen = TRUE ;
	                l = sprintf(cmd_lookbib,"lookbib %s 2>&1",
	                    database) ;

#if	DEBUG
	debugprintf("MAXPATHLEN=%d l=%d\n",MAXPATHLEN,l) ;

	for (ii = l + 1 ; ii < (MAXPATHLEN*2) ; ii += 1) {

		if (cmd_lookbib[ii] != 0x55) {

			debugprintf("got an error in the buffer at %d %02X\n",
				ii,cmd_lookbib[ii]) ;

		}

	}

	debugprintf("linebuf :\n") ;

	for (ii = 0 ; ii < len ; ii += 1)
		debugprintf(" %02X",linebuf[ii]) ;

	debugprintf("\n") ;
#endif
	                if (f_debug)
	                    bprintf(efp,"about to open the lookup program\n") ;

	                bflush(efp) ;

#if	DEBUG
	for (ii = 0 ; ii < len ; ii += 1)
		debugprintf(" %02X",linebuf[ii]) ;

	debugprintf("\n") ;
#endif
	                if ((pid_lookbib = bopencmd(fpa,cmd_lookbib)) < 0)
	                    goto badcmd ;

#if	DEBUG
	for (ii = 0 ; ii < len ; ii += 1)
		debugprintf(" %02X",linebuf[ii]) ;

	debugprintf("\n") ;
#endif
	                if (f_debug)
	                    bprintf(efp,"opened the lookup program\n") ;

/* read the initial prompt */

	                if (((rs = bread(lofp,&c1,1)) < 1) ||
	                    ((rs = bread(lofp,&c2,1)) < 1)) goto badresponse ;

#if	DEBUG
	for (ii = 0 ; ii < len ; ii += 1)
		debugprintf(" %02X",linebuf[ii]) ;

	debugprintf("\n") ;
#endif
	                if (f_debug) bprintf(efp,
	                    "read the initial response (%c) (%c)\n",
	                    c1,c2) ;

#if	DEBUG
	for (ii = 0 ; ii < len ; ii += 1)
		debugprintf(" %02X",linebuf[ii]) ;

	debugprintf("\n") ;
#endif
	            } /* end if (of opening the 'lookbib' program) */

	            if (f_debug)
	                bprintf(efp,"about to make a query\n") ;

/* make the query to the lookup program and record in output as a comment */

#if	DEBUG
	for (ii = 0 ; ii < len ; ii += 1)
		debugprintf(" %02X",linebuf[ii]) ;

	debugprintf("\n") ;
#endif
	            rbuf = linebuf + (macrolen + 1) ;
	            rlen = len - (macrolen + 1) ;

/* comment in output */

#if	DEBUG
	debugprintf("printing in output %d characters >\n%W",
	                len - (macrolen + 1),
			linebuf + (macrolen + 1),len - (macrolen + 1)) ;

	for (ii = 0 ; ii < (len - (macrolen + 1)) ; ii += 1)
		debugprintf(" %02X",(linebuf + (macrolen + 1))[ii]) ;

	debugprintf("\n") ;
#endif

	            bprintf(ofp,".\\\"_ %W",
	                linebuf + (macrolen + 1),len - (macrolen + 1)) ;

/* to program */

	            bwrite(lifp,
			linebuf + (macrolen + 1),len - (macrolen + 1)) ;

	            while (linebuf[len - 1] != '\n') {

#if	DEBUG
		debugprintf("getting more stuff off of input line\n") ;
#endif

	                if ((len = breadline(ifp,linebuf,LINELEN)) < 0)
	                    break ;

/* place a comment in the output consisting of the keywords given */

	                bprintf(ofp,".\\\"_ %W",
	                    linebuf,len) ;

/* to program */

	                bwrite(lifp,linebuf,len) ;

	            } /* end while */

	            bflush(lifp) ;

	            if (f_debug)
	                bprintf(efp,"about to read the response\n") ;

/* read the response */

	            init_br(&entry) ;

	            rs = OK ;
	            c2 = ' ' ;
	            while (TRUE) {

	                if (f_debug)
	                    bprintf(efp,"top of response loop\n") ;

	                if ((rs = bread(lofp,&c1,1)) < 0) break ;

	                if (f_debug)
	                    bprintf(efp,"read a character\n") ;

	                if ((c1 == '\n') || (c1 == '>')) break ;

	                if (c1 == '%') {

	                    if ((rs = bread(lofp,&c2,1)) < 0) break ;

	                    if (f_debug)
	                        bprintf(efp,"about to get the field line\n") ;

	                    if ((len = breadline(lofp,bibbuf,BIBLEN - 1)) < 1) {

	                        rs = (len < 0) ? len : BAD ;
	                        break ;
	                    }

	                    bo = 1 ;

	                } else {

	                    bibbuf[0] = c1 ;
	                    if ((len = 
	                        breadline(lofp,bibbuf + 1,BIBLEN - 2)) < 1) {

	                        rs = (len < 0) ? len : BAD ;
	                        break ;
	                    }

	                    len += 1 ;
	                    bo = 0 ;

	                }

	                f_bibeol = FALSE ;
	                if (bibbuf[len - 1] == '\n') f_bibeol = TRUE ;

	                if (f_debug)
	                    bprintf(efp,"about to switch\n") ;

	                switch ((int) c2) {

	                case 'A':
	                    add_bufdesc(&entry.a,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,TRUE) ;

	                    break ;

	                case 'T':
	                    add_bufdesc(&entry.t,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,FALSE) ;

	                    break ;

	                case 'D':
	                    add_bufdesc(&entry.d,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,FALSE) ;

	                    break ;

	                case 'J':
	                    add_bufdesc(&entry.j,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,FALSE) ;

	                    break ;

	                case 'M':
	                    add_bufdesc(&entry.m,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,FALSE) ;

	                    break ;

	                case 'I':
	                    add_bufdesc(&entry.i,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,FALSE) ;

	                    break ;

	                case 'P':
	                    add_bufdesc(&entry.p,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,FALSE) ;

	                    break ;

	                case 'C':
	                    add_bufdesc(&entry.c,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,FALSE) ;

	                    break ;

	                default:
	                    break ;

	                } /* end switch */

/* get rid of any remaining trash after our internal buffer limit */

	                while ((len > 0) && (bibbuf[len - 1] != '\n'))
	                    len = breadline(lofp,bibbuf,BIBLEN) ;

	            } /* end while (reading response) */

	            if (f_debug)
	                bprintf(efp,"out of the response while loop\n") ;

	            if (rs < 0) goto badresponse ;

/* clean up any trailing garbage from the 'lookbib' response */

	            if (f_debug) bprintf(efp,
	                "about to enter cleanup loop w/ C1 (%c)\n",c1) ;

	            f_double = FALSE ;
	            while (c1 != '>') {

	                if (c1 != '\n') {

	                    while (((len = 
	                        breadline(lofp,bibbuf,BIBLEN)) > 0) &&
	                        (bibbuf[len - 1] != '\n')) ;

	                    if ((rs = len) < 0) break ;

	                }

	                if ((rs = bread(lofp,&c1,1)) < 0) break ;

	                if (c1 == '%') f_double = TRUE ;

	            } /* end while (from cleaning up garbage) */

	            if (f_debug)
	                bprintf(efp,"about to read the final C2\n") ;

	            if ((rs = bread(lofp,&c2,1)) < 0) goto badresponse ;

/* write out the bibliographical reference in the correct format */

	            if (f_double) {

	                bprintf(ofp,
	                    "** more than one reference matched ") ;

	                bprintf(ofp,"the lookup key words **\n") ;

	                bprintf(ofp,".br\n%W.\\\"_\n",
	                    rbuf,rlen) ;

	            } else {

/* write out any authors first */

	                f_started = FALSE ;
	                if (entry.a.len > 0) {

	                    f_started = TRUE ;
	                    bprintf(ofp,"%W",entry.a.buf,entry.a.len) ;

	                }

	                f_needquoteend = FALSE ;
	                if (entry.t.len > 0) {

	                    if (f_started) bprintf(ofp,",\n") ;

	                    f_started = TRUE ;
	                    f_needquoteend = TRUE ;
	                    bprintf(ofp,"\"%W",
	                        entry.t.buf,entry.t.len) ;

	                }

	                if (entry.j.len > 0) {

	                    if (f_started) {

	                        if (f_needquoteend) {

	                            f_needquoteend = FALSE ;
	                            bprintf(ofp,",\"\n") ;

	                        } else
	                            bprintf(ofp,",\n") ;

	                    }

	                    f_started = TRUE ;
	                    bprintf(ofp,"%W",
	                        entry.j.buf,entry.j.len) ;

	                }

	                if (entry.m.len > 0) {

	                    if (f_started) {

	                        if (f_needquoteend) {

	                            f_needquoteend = FALSE ;
	                            bprintf(ofp,",\"\n") ;

	                        } else
	                            bprintf(ofp,",\n") ;

	                    }

	                    f_started = TRUE ;
	                    bprintf(ofp,"Bell Laboratories Memorandum %W",
	                        entry.m.buf,entry.m.len) ;

	                }

	                if (entry.i.len > 0) {

	                    if (f_started) {

	                        if (f_needquoteend) {

	                            f_needquoteend = FALSE ;
	                            bprintf(ofp,",\"\n") ;

	                        } else
	                            bprintf(ofp,",\n") ;

	                    }

	                    f_started = TRUE ;
	                    bprintf(ofp,"%W",
	                        entry.i.buf,entry.i.len) ;

	                }

	                if (entry.c.len > 0) {

	                    if (f_started) {

	                        if (f_needquoteend) {

	                            f_needquoteend = FALSE ;
	                            bprintf(ofp,",\"\n") ;

	                        } else
	                            bprintf(ofp,",\n") ;

	                    }

	                    f_started = TRUE ;
	                    bprintf(ofp,"%W",
	                        entry.c.buf,entry.c.len) ;

	                }

	                if (entry.d.len > 0) {

	                    if (f_started) {

	                        if (f_needquoteend) {

	                            f_needquoteend = FALSE ;
	                            bprintf(ofp,",\"\n") ;

	                        } else
	                            bprintf(ofp,",\n") ;

	                    }

	                    f_started = TRUE ;
	                    bprintf(ofp,"%W",
	                        entry.d.buf,entry.d.len) ;

	                }

	                if (entry.p.len > 0) {

	                    if (f_needquoteend) {

	                        f_needquoteend = FALSE ;
	                        bprintf(ofp,",\"\n") ;

	                    } else
	                        bprintf(ofp,",\n") ;

	                    f_started = TRUE ;
	                    bprintf(ofp,"pp %W",
	                        entry.p.buf,entry.p.len) ;

	                }

	                if (! f_started) {

	                    bprintf(ofp,
	                        "** not enough bibliographical ") ;

	                    bprintf(ofp,
	                        "information for a reference **\n") ;

	                    bprintf(ofp,".br\n%W.\\\"_\n",
	                        rbuf,rlen) ;

	                } else {

	                    if (f_needquoteend) bprintf(ofp,".\"\n") ;

	                    else bprintf(ofp,".\n") ;

	                }
	            }

	        } else 
	            bprintf(ofp,"** weird macro invocation encountered **\n") ;

/* whew, done with this reference */

	        if (f_debug)
	            bprintf(efp,"done with reference\n") ;

	    } else if ((rs = bwrite(ofp,linebuf,len)) < 0)
	        goto badwrite ;

	    f_begin = f_end ;

#if	DEBUG
	    if (f_debug)
	        bprintf(efp,"done with this line\n") ;
#endif

	} /* end while */

	if (f_debug)
	    bprintf(efp,"out of it now\n") ;

	if (f_lookopen) {

	    for (i = 0 ; i < 3 ; i += 1)
	        if (fpa[i] != NULL) bclose(fpa[i]) ;

	}

	bclose(ofp) ;

	bclose(ifp) ;

done:
	bclose(efp) ;

	return OK ;

badret:
	bclose(efp) ;

	return BAD ;

usage:
	bprintf(efp,
	    "usage: %s [infile] [-?VD]\n",
	    progname) ;

	bprintf(efp,
		"\tmusage macro is 'BK'\n") ;

	goto badret ;

badarg:
	bprintf(efp,"%s: bad argument given\n",progname) ;

	goto badret ;

badinfile:
	bprintf(efp,"%s: cannot open the input file (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badoutopen:
	bprintf(efp,"%s: could not open standard output (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badwrite:
	bclose(ifp) ;

	bclose(ofp) ;

	bprintf(efp,
	    "%s: could not perform the 'bwrite' to file system (rs %d)\n",
	    progname,rs) ;

	goto badret ;

badcmd:
	bclose(ifp) ;

	bclose(ofp) ;

	bprintf(efp,
	    "%s: could not open communication to 'lookbib' (rs %d)\n",
	    progname,pid_lookbib) ;

	goto badret ;

badargnum:
	bprintf(efp,"%s: not enough arguments specified\n",progname) ;

	goto badret ;

baddatabase:
	bprintf(efp,
	    "%s: could not read the bibliographical database \"%s\"\n",
	    progname,database) ;

	goto badret ;

badresponse:
	bprintf(efp,
	    "%s: got an error while reading response (rs %d)\n",
	    progname,rs) ;

	if (f_lookopen) {

	    for (i = 0 ; i < 3 ; i += 1)
	        if (fpa[i] != NULL) bclose(fpa[i]) ;

	}

	bclose(ofp) ;

	bclose(ifp) ;

	goto badret ;
}
/* end subroutine (main) */


/* initialize a bibliographical reference */

void init_bufdesc(bdp)
struct bufdesc	*bdp ;
{

	bdp->buf[0] = '\0' ;
	bdp->len = 0 ;
}
/* end subroutine (init_bufdesc) */


int add_bufdesc(bdp,buf,len,f_comma)
struct bufdesc	*bdp ;
char	buf[] ;
int	len, f_comma ;
{
	int	rs = OK ;

	char	*w ;


	if ((BIBLEN - bdp->len) < 3) return BAD ;

	if (len > (BIBLEN - bdp->len - 3)) {

	    len = (BIBLEN - bdp->len) ;
	    rs = 1 ;
	}

	if (bdp->len > 0) {

	    w = (f_comma) ? ", " : " " ;
	    strcpy(bdp->buf + bdp->len,w) ;

	    bdp->len += strlen(w) ;
	}

	strncpy(bdp->buf + bdp->len,buf,len) ;

	bdp->len += len ;
	bdp->buf[bdp->len] = '\0' ;
	return rs ;
}
/* end subroutine (add_bufdesc) */


void init_br(brp)
struct br	*brp ;
{

	init_bufdesc(&brp->a) ;

	init_bufdesc(&brp->t) ;

	init_bufdesc(&brp->d) ;

	init_bufdesc(&brp->j) ;

	init_bufdesc(&brp->m) ;

	init_bufdesc(&brp->i) ;

	init_bufdesc(&brp->p) ;

	init_bufdesc(&brp->c) ;

}
/* end subroutine */




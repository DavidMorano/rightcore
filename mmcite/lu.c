/* lu */

/* lookup a reference in the databases */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time */
#define	CF_DEBUG	0		/* run-time */


/* revision history:

	= 1992-03-10, David A­D­ Morano
	This subroutine was originally written.

	= 1998-09-10, David A­D­ Morano
        This subroutine was modified to be able to handle the response from the
        GNU 'lookbib' program in addition to the (old) standard UNIX version.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine processes a file by looking up and inserting the
        bibliographical references into the text. All input is copied to the
        output with the addition of the bibliographical references.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>

#include	<bfile.h>
#include	<localmisc.h>

#include	"bdb.h"
#include	"lu.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	CMDBUFLEN
#define	CMDBUFLEN	(2 * MAXPATHLEN)
#endif


/* external subroutines */


/* external variables */


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


/* forward subroutine references */

void		bufdesc_init(), br_init() ;


/* exported subroutines */


int lu_open(bdbp)
bdb	*bdbp ;
{
	if (bdbp == NULL) return SR_FAULT ;
	memset(bdbp,0,sizeof(LU)) ;
	return SR_OK ;
}
/* end subroutine (lu_open) */


int lookup(gp,bdbp,keys)
struct global	*gp ;
bdb		*bdbp ;
char		keys[] ;
{
	bfile		infile, *ifp = &infile ;
	bfile		lookifile, *lifp = &lookifile ;
	bfile		lookofile, *lofp = &lookofile ;
	bfile		lookefile, *lefp = &lookefile ;
	bfile		*fpa[3] ;

	struct br	entry ;

	pid_t		pid_lookbib ;

	int	rs ;
	int	i ;
	int	l, len ;
	int	macrolen ;
	int	macrolen1 = strlen(MACRONAME1) ;
	int	macrolen2 = strlen(MACRONAME2) ;
	int	bo ;
	int	rlen ;
	int	ii ;
	int	f_begin = TRUE ;
	int	f_end ;
	int	f_pass ;
	int	f_lookopen = FALSE ;
	int	f_checkangle, f_sawangle, f_done, f_record ;
	int	f_double ;
	int	f_bibeol ;
	int	f_needquoteend ;
	int	f_started ;

	char	*database ;
	char	linebuf[LINELEN + 1] ;
	char	bibbuf[BIBLEN + 1] ;
	char	cmd_lookbib[CMDBUFLEN + 1] ;
	char	*cp, *rbuf ;
	char	c, ch1, ch2 ;


#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("process: entered\n") ;
#endif

	bdbgetkey(bdbp,NULL,&database) ;

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("process: db=%s\n",database) ;
#endif

#if	CF_DEBUG
	if (gp->debuglevel > 1) {
	    BDB_CURSOR	cur ;
	    bdbnullcursor(&cur) ;
	    while (bdbgetkey(bdbp,&cur,&cp) >= 0) {
	        if (cp == NULL) continue ;
	        debugprintf("process: db=%s\n",cp) ;
	    } /* end for */
	    bdbnullcursor(&cur) ;
	}
#endif /* CF_DEBUG */

/* open files */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("process: about to open files\n") ;
#endif

	if ((filename == NULL) || (filename[0] == '-')) {
	    rs = bopen(ifp,BFILE_STDIN,"dr",0666) ;

	} else
	    rs = bopen(ifp,filename,"r",0666) ;

	if (rs < 0)
	    goto badinfile ;

/* other initializations */

	fpa[0] = lifp ;
	fpa[1] = lofp ;
	fpa[2] = NULL ;

/* go through the loops */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("process: 1 above while loop\n") ;
#endif

	f_begin = TRUE ;
	while ((len = breadline(ifp,linebuf,LINELEN)) > 0) {

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process: top while \n") ;
#endif

	    f_end = TRUE ;
	    if (linebuf[len - 1] != '\n') f_end = FALSE ;

#if	CF_DEBUG
	    if (gp->debuglevel > 1)
	        debugprintf("process: processing a line begin=%d end=%d\n%W",
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

#if	CF_DEBUG
	        if (gp->debuglevel > 1) {

	            debugprintf("process: input line >\n%W",
	                linebuf,len) ;

#ifdef	COMMENT
	            for (ii = 0 ; ii < len ; ii += 1)
	                debugprintf(" %02X",linebuf[ii]) ;
#endif

	            debugprintf("\n") ;

	        }
#endif /* CF_DEBUG */

	        if (len > (macrolen + 1)) {

#if	CF_DEBUG
	            if (gp->debuglevel > 1)
	                debugprintf("process: len _gt_ something\n") ;
#endif

	            if (gp->debuglevel > 0)
	                bprintf(gp->efp,"%s: got a bibliographical reference\n",
	                    gp->progname) ;

#if	CF_DEBUG
	            if (gp->debuglevel > 1) {

	                debugprintf("process: line\n") ;

	                for (ii = 0 ; ii < len ; ii += 1)
	                    debugprintf(" %02X",linebuf[ii]) ;

	                debugprintf("\n") ;

	            }
#endif /* CF_DEBUG */

	            if (! f_lookopen) {

#if	CF_DEBUG
	                if (gp->debuglevel > 1)
	                    debugprintf("process: not f_lookopen\n") ;
#endif

#if	CF_DEBUG
	                if (gp->debuglevel > 1) {

	                    for (ii = 0 ; ii < len ; ii += 1)
	                        debugprintf(" %02X",linebuf[ii]) ;

	                    debugprintf("\n") ;

	                    if (database != NULL)
	                        debugprintf("process: database %d > %s\n",
	                            strlen(database),database) ;

	                    else
	                        debugprintf("process: NULL database\n") ;

	                    for (ii = 0 ; ii < strlen(database) ; ii += 1)
	                        debugprintf(" %02X",database[ii]) ;

	                    for (ii = 0 ; ii < (MAXPATHLEN*2) ; ii += 1)
	                        cmd_lookbib[ii] = 0x55 ;

	                    debugprintf("\n") ;

	                }
#endif /* CF_DEBUG */

#if	CF_DEBUG
	                if (gp->debuglevel > 1)
				debugprintf("process: lookbib=%s\n",
				gp->prog_lookbib) ;
#endif /* CF_DEBUG */

	                f_lookopen = TRUE ;
	                l = bufprintf(cmd_lookbib,CMDBUFLEN,"%s %s 2>&1",
	                    gp->prog_lookbib,database) ;

#if	CF_DEBUG
	                if (gp->debuglevel > 1) {

	                    debugprintf("process: MAXPATHLEN=%d l=%d\n",
	                        MAXPATHLEN,l) ;

	                    for (ii = l + 1 ; ii < (MAXPATHLEN*2) ; ii += 1) {

	                        if (cmd_lookbib[ii] != 0x55) {

	                            debugprintf("process: error buffer %d %02X\n",
	                                ii,cmd_lookbib[ii]) ;

	                        }

	                    } /* end for */

	                    debugprintf("process: linebuf :\n") ;

	                    for (ii = 0 ; ii < len ; ii += 1)
	                        debugprintf(" %02X",linebuf[ii]) ;

	                    debugprintf("\n") ;

	                }
#endif /* CF_DEBUG */

	                if (gp->debuglevel > 0)
	                    bprintf(gp->efp,
	                        "%s: about to open the lookup program\n",
	                        gp->progname) ;

	                bflush(gp->efp) ;

#if	CF_DEBUG
	                if (gp->debuglevel > 1) {

	                    for (ii = 0 ; ii < len ; ii += 1)
	                        debugprintf(" %02X",linebuf[ii]) ;

	                    debugprintf("\n") ;

	                }
#endif /* CF_DEBUG */

	                if ((pid_lookbib = bopencmd(fpa,cmd_lookbib)) < 0)
	                    goto badcmd ;

#if	CF_DEBUG
	                if (gp->debuglevel > 1) {

	                    for (ii = 0 ; ii < len ; ii += 1)
	                        debugprintf(" %02X",linebuf[ii]) ;

	                    debugprintf("\n") ;

	                }
#endif /* CF_DEBUG */

#if	CF_DEBUG
	                if (gp->debuglevel > 1)
	                    debugprintf("process: opened the lookup program\n") ;
#endif

/* read the initial prompt */

	                if (((rs = bread(lofp,&ch1,1)) < 1) ||
	                    ((rs = bread(lofp,&ch2,1)) < 1))
	                    goto badresponse ;

#if	CF_DEBUG
	                if (gp->debuglevel > 1) {

	                    for (ii = 0 ; ii < len ; ii += 1)
	                        debugprintf(" %02X",linebuf[ii]) ;

	                    debugprintf("\n") ;

	                }
#endif /* CF_DEBUG */

#if	CF_DEBUG
	                if (gp->debuglevel > 1)
	                    debugprintf(
	                        "process: read initial response (%c) (%c)\n",
	                        ch1,ch2) ;
#endif

#if	CF_DEBUG
	                if (gp->debuglevel > 1) {

	                    for (ii = 0 ; ii < len ; ii += 1)
	                        debugprintf(" %02X",linebuf[ii]) ;

	                    debugprintf("\n") ;

	                }
#endif /* CF_DEBUG */

	            } /* end if (of opening the 'lookbib' program) */

#if	CF_DEBUG
	            if (gp->debuglevel > 1)
	                debugprintf("process: about to make query\n") ;
#endif

	            if (gp->debuglevel > 0)
	                bprintf(gp->efp,"%s: about to make a query\n",
	                    gp->progname) ;

/* make the query to the lookup program and record in output as a comment */

#if	CF_DEBUG
	            if (gp->debuglevel > 1) {

	                for (ii = 0 ; ii < len ; ii += 1)
	                    debugprintf(" %02X",linebuf[ii]) ;

	                debugprintf("\n") ;

	            }
#endif /* CF_DEBUG */

	            rbuf = linebuf + (macrolen + 1) ;
	            rlen = len - (macrolen + 1) ;

/* comment in output */

#if	CF_DEBUG
	            if (gp->debuglevel > 1) {

	                debugprintf(
	                    "process: printing in output %d characters >\n%W",
	                    len - (macrolen + 1),
	                    linebuf + (macrolen + 1),len - (macrolen + 1)) ;

	                for (ii = 0 ; ii < (len - (macrolen + 1)) ; ii += 1)
	                    debugprintf(" %02X",(linebuf + (macrolen + 1))[ii]) ;

	                debugprintf("\n") ;

	            }
#endif /* CF_DEBUG */

	            bprintf(ofp,".\\\"_ %W",
	                linebuf + (macrolen + 1),len - (macrolen + 1)) ;

/* to program */

	            bwrite(lifp,
	                linebuf + (macrolen + 1),len - (macrolen + 1)) ;

	            while (linebuf[len - 1] != '\n') {

#if	CF_DEBUG
	                if (gp->debuglevel > 1)
	                    debugprintf("process: more off of input line\n") ;
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

	            if (gp->debuglevel > 0)
	                bprintf(gp->efp,"%s: about to read the response\n",
	                    gp->progname) ;

/* read the response */

	            br_init(&entry) ;

	            rs = OK ;
	            ch2 = ' ' ;
	            while (TRUE) {

#if	CF_DEBUG
	                if (gp->debuglevel > 1)
	                    debugprintf("process: top of response loop\n") ;
#endif

	                if ((rs = bread(lofp,&ch1,1)) < 0) break ;

#if	CF_DEBUG
	                if (gp->debuglevel > 1)
	                    debugprintf("process: read a character\n") ;
#endif /* CF_DEBUG */

	                if ((ch1 == '\n') || (ch1 == '>'))
				break ;

	                if (ch1 == '%') {

	                    if ((rs = bread(lofp,&ch2,1)) < 0) break ;

	                    if (gp->debuglevel > 1)
	                        debugprintf(
	                            "process: about to get the field line\n") ;

	                    if ((len = breadline(lofp,bibbuf,BIBLEN - 1)) < 1) {

	                        rs = (len < 0) ? len : BAD ;
	                        break ;
	                    }

	                    bo = 1 ;

	                } else {

	                    bibbuf[0] = ch1 ;
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

#if	CF_DEBUG
	                if (gp->debuglevel > 1)
	                    debugprintf("process: about to switch\n") ;
#endif

	                switch ((int) ch2) {

	                case 'A':
	                    bufdesc_add(&entry.a,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,TRUE) ;

	                    break ;

	                case 'T':
	                    bufdesc_add(&entry.t,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,FALSE) ;

	                    break ;

	                case 'D':
	                    bufdesc_add(&entry.d,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,FALSE) ;

	                    break ;

	                case 'J':
	                    bufdesc_add(&entry.j,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,FALSE) ;

	                    break ;

	                case 'M':
	                    bufdesc_add(&entry.m,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,FALSE) ;

	                    break ;

	                case 'I':
	                    bufdesc_add(&entry.i,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,FALSE) ;

	                    break ;

	                case 'P':
	                    bufdesc_add(&entry.p,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,FALSE) ;

	                    break ;

	                case 'C':
	                    bufdesc_add(&entry.c,bibbuf + bo,
	                        ((f_bibeol) ? len - 1 : len) - bo,FALSE) ;

	                    break ;

	                default:
	                    break ;

	                } /* end switch */

/* get rid of any remaining trash after our internal buffer limit */

	                while ((len > 0) && (bibbuf[len - 1] != '\n'))
	                    len = breadline(lofp,bibbuf,BIBLEN) ;

	            } /* end while (reading response) */

#if	CF_DEBUG
	            if (gp->debuglevel > 1)
	                debugprintf("process: out of the response while loop\n") ;
#endif

	            if (rs < 0) goto badresponse ;

/* clean up any trailing garbage from the 'lookbib' response */

#if	CF_DEBUG
	            if (gp->debuglevel > 1)
	                debugprintf(
	                    "process: about to enter cleanup loop w/ C1 (%c)\n",
	                    ch1) ;
#endif

	            f_double = FALSE ;
	            while (ch1 != '>') {

	                if (ch1 != '\n') {

	                    while (((len = 
	                        breadline(lofp,bibbuf,BIBLEN)) > 0) &&
	                        (bibbuf[len - 1] != '\n')) ;

	                    if ((rs = len) < 0) break ;

	                }

	                if ((rs = bread(lofp,&ch1,1)) < 0) break ;

	                if (ch1 == '%') f_double = TRUE ;

	            } /* end while (from cleaning up garbage) */

#if	CF_DEBUG
	            if (gp->debuglevel > 1)
	                debugprintf("process: about to read the final C2\n") ;
#endif

	            if ((rs = bread(lofp,&ch2,1)) < 0) goto badresponse ;

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

/* titles */

	                f_needquoteend = FALSE ;
	                if (entry.t.len > 0) {

	                    if (f_started) bprintf(ofp,",\n") ;

	                    f_started = TRUE ;
	                    if (entry.j.len > 0) {

	                        f_needquoteend = TRUE ;
	                        bprintf(ofp,"\"%W",
	                            entry.t.buf,entry.t.len) ;

	                    } else {

	                        bprintf(ofp,"\\fI%W\\fP",
	                            entry.t.buf,entry.t.len) ;

	                    } /* end if (journal or not) */

	                } /* end if (title) */
/* journal */

	                if (entry.j.len > 0) {

	                    if (f_started) {

	                        if (f_needquoteend) {

	                            f_needquoteend = FALSE ;
	                            bprintf(ofp,",\"\n") ;

	                        } else
	                            bprintf(ofp,",\n") ;

	                    }

	                    f_started = TRUE ;
	                    bprintf(ofp,"\\fI%W\\fP",
	                        entry.j.buf,entry.j.len) ;

	                }

/* memorandum */

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

/* issuer (publisher) */

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

/* publisher's address */

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

/* date */

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

	        if (gp->debuglevel > 0)
	            bprintf(gp->efp,"done with reference\n") ;

	    } else if ((rs = bwrite(ofp,linebuf,len)) < 0)
	        goto badwrite ;

	    f_begin = f_end ;

#if	CF_DEBUG
	    if (gp->debuglevel > 0)
	        bprintf(gp->efp,"done with this line\n") ;
#endif

	} /* end while (reading input lines) */

#if	CF_DEBUG
	if (gp->debuglevel > 1)
	    debugprintf("process: out of it now\n") ;
#endif

	if (f_lookopen) {

	    for (i = 0 ; i < 3 ; i += 1)
	        if (fpa[i] != NULL) bclose(fpa[i]) ;

	}

	bclose(ofp) ;

	bclose(ifp) ;

done:
	bclose(gp->efp) ;

	return OK ;

badret:
	bclose(gp->efp) ;

	return BAD ;

usage:
	bprintf(gp->efp,
	    "%s: USAGE> %s [infile] [-?VD]\n",
	    gp->progname,gp->progname) ;

	bprintf(gp->efp,
	    "\tmusage macro is 'BK'\n") ;

	goto badret ;

badarg:
	bprintf(gp->efp,"%s: bad argument given\n",gp->progname) ;

	goto badret ;

badinfile:
	bprintf(gp->efp,"%s: cannot open the input file (rs %d)\n",
	    gp->progname,rs) ;

	goto badret ;

badoutfile:
	bprintf(gp->efp,"%s: could not open standard output (rs %d)\n",
	    gp->progname,rs) ;

	goto badret ;

badwrite:
	bclose(ifp) ;

	bclose(ofp) ;

	bprintf(gp->efp,
	    "%s: could not perform the 'bwrite' to file system (rs %d)\n",
	    gp->progname,rs) ;

	goto badret ;

badcmd:
	bclose(ifp) ;

	bclose(ofp) ;

	bprintf(gp->efp,
	    "%s: could not open communication to 'lookbib' (rs %d)\n",
	    gp->progname,pid_lookbib) ;

	goto badret ;

badargnum:
	bprintf(gp->efp,"%s: not enough arguments specified\n",
	    gp->progname) ;

	goto badret ;

baddatabase:
	bprintf(gp->efp,
	    "%s: could not read the bibliographical database \"%s\"\n",
	    gp->progname,database) ;

	goto badret ;

badresponse:
	bprintf(gp->efp,
	    "%s: got an error while reading response (rs %d)\n",
	    gp->progname,rs) ;

	if (f_lookopen) {

	    for (i = 0 ; i < 3 ; i += 1)
	        if (fpa[i] != NULL) bclose(fpa[i]) ;

	}

	bclose(ofp) ;

	bclose(ifp) ;

	goto badret ;
}
/* end subroutine (lookup) */


/* initialize a bibliographical reference */

void bufdesc_init(bdp)
struct bufdesc	*bdp ;
{

	bdp->buf[0] = '\0' ;
	bdp->len = 0 ;
}
/* end subroutine (bufdesc_init) */


int bufdesc_add(bdp,buf,len,f_comma)
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
/* end subroutine (bufdesc_add) */


void br_init(brp)
struct br	*brp ;
{


	bufdesc_init(&brp->a) ;

	bufdesc_init(&brp->t) ;

	bufdesc_init(&brp->d) ;

	bufdesc_init(&brp->j) ;

	bufdesc_init(&brp->m) ;

	bufdesc_init(&brp->i) ;

	bufdesc_init(&brp->p) ;

	bufdesc_init(&brp->c) ;

}
/* end subroutine (br_init) */




/* filewatch */

/* watch a file for changes and report */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-07-10, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This module watches a single file for changes and prints out the
        trailing changes to a file.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<ascii.h>
#include	<localmisc.h>

#include	"filewatch.h"
#include	"strfilter.h"
#include	"linefold.h"


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	CLEANBUFLEN	LINEBUFLEN

#define	TO_OPEN		60

#ifndef	STDINFNAME
#define	STDINFNAME	"*STDIN*"
#endif

#ifndef	STDOUTFNAME
#define	STDOUTFNAME	"*STDOUT*"
#endif

#ifndef	STDERRFNAME
#define	STDERRFNAME	"*STDERR*"
#endif


/* external subroutines */

extern int	isprintlatin(int) ;


/* external variables */


/* local structures */


/* forward references */

static int	filewatch_fileopen(FILEWATCH *,time_t,const char *,int) ;
static int	filewatch_fileclose(FILEWATCH *) ;
static int	filewatch_putout(FILEWATCH *,bfile *,char *,int) ;
static int	filewatch_putoutline(FILEWATCH *,bfile *,int,cchar *,int) ;

static int	mkclean(char *,int,const char *,int) ;
static int	hasourbad(const char *,int) ;
static int	isourbad(int) ;


/* local variables */


/* exported subroutines */


int filewatch_start(op,ap,sfp,fname)
FILEWATCH	*op ;
FILEWATCH_ARGS	*ap ;
STRFILTER	*sfp ;
const char	fname[] ;
{
	time_t		daytime = time(NULL) ;
	int		rs = SR_OK ;
	int		size ;

	if (op == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	memset(op,0,sizeof(FILEWATCH)) ;

#if	CF_DEBUGS
	debugprintf("filewatch_start: fname=%s\n",fname) ;
#endif

	op->sfp = sfp ;
	size = FILEWATCH_BUFLEN ;
	rs = uc_malloc(size,&op->buf) ;
	if (rs < 0)
	    goto bad0 ;

	op->lastcheck = daytime ;
	rs = filewatch_fileopen(op,daytime,fname,-1) ;
	if (rs < 0)
	    goto bad1 ;

/* save the file name */

	rs = uc_mallocstrw(fname,-1,&op->fname) ;
	if (rs < 0)
	    goto bad2 ;

	op->interval = (ap->interval > 1) ? ap->interval : 1 ;
	op->cut = (ap->cut > 0) ? ap->cut : 0 ;
	op->columns = MAX(ap->columns,3) ;
	op->indent = MAX(ap->indent,0) ;
	op->opts = ap->opts ;

#if	CF_DEBUGS
	debugprintf("filewatch_start: cut=%d\n",op->cut) ;
#endif

	op->f.carriage = (ap->opts & FILEWATCH_MCARRIAGE) ;
	op->f.clean = (ap->opts & FILEWATCH_MCLEAN) ;

ret0:

#if	CF_DEBUGS
	debugprintf("filewatch_start: ret OK\n") ;
#endif

	return rs ;

/* bad stuff */
bad2:
	bclose(&op->wfile) ;

bad1:
	if (op->buf != NULL) {
	    uc_free(op->buf) ;
	    op->buf = NULL ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (filewatch_start) */


int filewatch_finish(op)
FILEWATCH	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op == NULL) return SR_FAULT ;

	if (op->fname != NULL) {
	    rs1 = uc_free(op->fname) ;
	    if (rs >= 0) rs = rs1 ;
	    op->fname = NULL ;
	}

	if (op->f.open) {
	    op->f.open = FALSE ;
	    rs1 = bclose(&op->wfile) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->buf != NULL) {
	    rs1 = uc_free(op->buf) ;
	    if (rs >= 0) rs = rs1 ;
	    op->buf = NULL ;
	}

	return rs ;
}
/* end subroutine (filewatch_finish) */


/* check if our file has changed */
int filewatch_check(op,daytime,ofp)
FILEWATCH	*op ;
time_t		daytime ;
bfile		*ofp ;
{
	struct ustat	sb ;
	offset_t	boff ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		len ;
	int		wlen = 0 ;
	int		f ;

	if (op == NULL) return SR_FAULT ;

	if ((daytime - op->lastcheck) < op->interval)
	    goto ret0 ;

	op->lastcheck = daytime ;

/* check if the modification time or the size of the file has changed */

	if (op->f.open) {
	    rs = bcontrol(&op->wfile,BC_STAT,&sb) ;
	} else
	    rs = u_stat(op->fname,&sb) ;

	if (rs < 0)
	    goto bad1 ;

	if ((sb.st_mtime > op->lastchange) || (sb.st_size != op->offset)) {

	    if (! op->f.open) {

	        rs = filewatch_fileopen(op,daytime,op->fname,op->offset) ;
	        if (rs < 0)
	            goto bad2 ;

	    } /* end if (file open?) */

/* try to process some lines in the file */

	    if (sb.st_size > op->offset) {

/* CONSTCOND */

	        while (TRUE) {
	            int		blen ;
	            char	*bp ;

	            bp = op->buf + op->bi ;
	            blen = FILEWATCH_BUFLEN - op->bi ;
	            rs = breadline(&op->wfile,bp,blen) ;
	            len = rs ;
	            if (rs <= 0)
	                break ;

#if	CF_DEBUGS
	            debugprintf("filewatch_check: part line>%t<\n",
	                bp,
	                ((bp[len - 1] == '\n') ? (len - 1) : len)) ;
#endif

	            op->bi += len ;
	            if ((len >= FILEWATCH_BUFLEN) ||
	                (op->buf[op->bi - 1] == '\n')) {

#if	CF_DEBUGS
	                debugprintf("filewatch_check: got EOL\n",
	                    bp,len) ;
#endif

	                f = TRUE ;
	                if (op->sfp != NULL) {
	                    rs1 = strfilter_check(op->sfp,op->buf,op->bi) ;
	                    f = (rs1 > 0) ;
	                } /* end if */

	                if (f) {
	                    rs = filewatch_putout(op,ofp,op->buf,op->bi) ;
	                    wlen += rs ;
	                    if (rs < 0)
	                        break ;
	                } /* end if */

	                op->bi = 0 ;

	            } /* end if (any output needed) */

	            op->offset += len ;

	        } /* end while (processing lines) */

	    } /* end if (change) */

	    if (sb.st_size < op->offset) {

#if	CF_DEBUGS
	        debugprintf("filewatch_check: got a size reduction\n") ;
#endif

		boff = sb.st_size ;
	        bseek(&op->wfile,boff,SEEK_SET) ;

	        op->offset = sb.st_size ;
	    }

	    op->lastchange = daytime ;

	} /* end if (change of some kind) */

/* is the file still present in the file system? */

	if ((rs >= 0) && op->f.open && ((daytime - op->opentime) > TO_OPEN)) {

	    int	f ;


	    f = (strcmp(op->fname,"-") == 0) || 
	        (strcmp(op->fname,STDINFNAME) == 0) ;

	    if (! f) {

	        rs = u_stat(op->fname,&sb) ;

	        if ((rs == SR_NOENT) || ((rs >= 0) &&
	            ((sb.st_ino != op->ino) || (sb.st_dev != op->dev)))) {

	            rs = filewatch_fileclose(op) ;

	            if (rs >= 0)
	                rs = SR_NOENT ;

	        }

	    } /* end if */

	} /* end if (file-open check) */

ret0:
	return (rs >= 0) ? wlen : rs ;

/* bad things */
bad2:
bad1:
	if (op->f.open) {
	    op->f.open = FALSE ;
	    bclose(&op->wfile) ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (filewatch_check) */


/* read a line */
int filewatch_readline(op,daytime,linebuf,linelen)
FILEWATCH	*op ;
time_t		daytime ;
char		linebuf[] ;
int		linelen ;
{
	struct ustat	sb ;
	offset_t	boff ;
	int		rs = SR_OK ;
	int		len, ml ;
	int		wlen = 0 ;

	if (op == NULL) return SR_FAULT ;
	if (linebuf == NULL) return SR_FAULT ;

	if (linelen < 0) return SR_INVALID ;

	if (linelen == 0)
	    goto ret0 ;

	if (op->ll > 0) {

	    ml = MIN(linelen,op->ll) ;
	    memcpy(linebuf,op->lp,ml) ;

	    op->ll -= ml ;
	    wlen = ml ;
	    goto ret0 ;
	}

/* continue to ret to read a line */

	if ((daytime - op->lastcheck) < op->interval)
	    goto ret0 ;

	op->lastcheck = daytime ;

/* check if the modification time or the size of the file has changed */

	if (op->f.open)
	    rs = bcontrol(&op->wfile,BC_STAT,&sb) ;

	else
	    rs = u_stat(op->fname,&sb) ;

	if (rs < 0)
	    goto bad1 ;

	if ((sb.st_mtime > op->lastchange) || (sb.st_size != op->offset)) {

	    if (! op->f.open) {

	        rs = filewatch_fileopen(op,daytime,
	            op->fname,op->offset) ;

	        if (rs < 0)
	            goto bad2 ;

	    } /* end if (file open?) */

/* try to process some lines in the file */

	    if (sb.st_size > op->offset) {

	        int		blen ;

	        char	*bp ;


	        bp = op->buf + op->bi ;
	        blen = FILEWATCH_BUFLEN - op->bi ;
	        rs = breadline(&op->wfile,bp,blen) ;
	        len = rs ;
	        if ((rs >= 0) && (len > 0)) {

#if	CF_DEBUGS
	            debugprintf("filewatch_check: part line> %t\n",
	                bp,len) ;
#endif

	            op->offset += len ;
	            op->bi += len ;
	            if ((len >= FILEWATCH_BUFLEN) ||
	                (op->buf[op->bi - 1] == '\n')) {

	                int	ml ;


#if	CF_DEBUGS
	                debugprintf("filewatch_check: got EOL\n",
	                    bp,len) ;
#endif

	                ml = MIN(linelen,op->bi) ;
	                memcpy(linebuf,op->buf,ml) ;

	                wlen += ml ;
	                op->lp = op->buf + ml ;
	                op->ll = op->bi - ml ;
	                op->bi = 0 ;

	            } /* end if (got a line) */

	        } /* end if (got some data) */

	    } else if (sb.st_size < op->offset) {

#if	CF_DEBUGS
	        debugprintf("filewatch_check: got a size reduction\n") ;
#endif

		boff = sb.st_size ;
	        bseek(&op->wfile,boff,SEEK_SET) ;

	        op->offset = sb.st_size ;
	    }

	    op->lastchange = daytime ;

	} /* end if (change of some kind) */

/* is the file still present in the file system? */

	if ((rs >= 0) && op->f.open && ((daytime - op->opentime) > TO_OPEN)) {
	    int	f ;

	    f = (strcmp(op->fname,"-") == 0) || 
	        (strcmp(op->fname,STDINFNAME) == 0) ;

	    if (! f) {

	        rs = u_stat(op->fname,&sb) ;

	        if ((rs == SR_NOENT) || ((rs >= 0) &&
	            ((sb.st_ino != op->ino) || (sb.st_dev != op->dev)))) {

	            rs = filewatch_fileclose(op) ;

	            if (rs >= 0)
	                rs = SR_NOENT ;

	        }

	    } /* end if */

	} /* end if (file-open check) */

ret0:
	return (rs >= 0) ? wlen : rs ;

/* bad things */
bad2:
bad1:
	if (op->f.open) {
	    op->f.open = FALSE ;
	    bclose(&op->wfile) ;
	}

bad0:
	goto ret0 ;
}
/* end subroutine (filewatch_readline) */


/* private subroutines */


static int filewatch_fileopen(op,daytime,fname,off)
FILEWATCH	*op ;
time_t		daytime ;
const char	fname[] ;
int		off ;
{
	struct ustat	sb ;
	offset_t	boff ;
	offset_t	offset ;
	int		rs = SR_OK ;

	if (op->f.open)
	    goto ret0 ;

	if ((strcmp(fname,"-") != 0) && (strcmp(fname,STDINFNAME) != 0)) {
	    rs = bopen(&op->wfile,fname,"r",0666) ;
	} else
	    rs = bopen(&op->wfile,BFILE_STDIN,"r",0666) ;

	if (rs < 0)
	    goto bad0 ;

	op->f.open = TRUE ;
	bcontrol(&op->wfile,BC_LINEBUF,0) ;

	op->offset = off ;
	if (off < 0) {

	    bseek(&op->wfile,0L,SEEK_END) ;

	    btell(&op->wfile,&boff) ;
	    offset = boff ;

	    op->offset = offset ;

	} else {
	    boff = off ;
	    bseek(&op->wfile,boff,SEEK_SET) ;
	}

/* other stuff */

	bcontrol(&op->wfile,BC_STAT,&sb) ;

	op->lastchange = sb.st_mtime ;
	op->ino = sb.st_ino ;
	op->dev = sb.st_dev ;

	op->opentime = daytime ;

bad0:
ret0:
	return rs ;
}
/* end subroutine (filewatch_fileopen) */


static int filewatch_fileclose(op)
FILEWATCH	*op ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (op->f.open) {
	    op->f.open = FALSE ;
	    rs1 = bclose(&op->wfile) ;
	    if (rs >= 0) rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (filewatch_fileclose) */


/* put out a line to the output */
static int filewatch_putout(op,ofp,buf,blen)
FILEWATCH	*op ;
bfile		*ofp ;
char		buf[] ;
int		blen ;
{
	LINEFOLD	liner ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		ll, sl ;
	int		i ;
	int		wlen = 0 ;
	int		f_eol ;
	const char	*lp ;
	const char	*sp ;

#if	CF_DEBUGS
	debugprintf("filewatch_putout: buf=>%t<\n",
	    buf,((buf[blen - 1] == '\n') ? (blen - 1) : blen)) ;
#endif

	if (blen <= op->cut)
	    goto ret0 ;

#if	CF_DEBUGS
	debugprintf("filewatch_putout: beyond cutoff blen=%d\n",
	    blen) ;
#endif

	lp = buf + op->cut ;
	ll = blen - op->cut ;
	if (ll <= 0)
	    goto ret0 ;

	f_eol = (lp[ll - 1] == '\n') ;
	if ((rs = linefold_start(&liner,op->columns,op->indent,lp,ll)) >= 0) {

	    for (i = 0 ; (sl = linefold_get(&liner,i,&sp)) >= 0 ; i += 1) {
	        if (sl == 0) continue ;

	        rs = filewatch_putoutline(op,ofp,i,sp,sl) ;
	        wlen += rs ;
	        if (rs < 0)
	            break ;

	    } /* end for */

	    rs1 = linefold_finish(&liner) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (linefold) */

	if (rs >= 0)
	    rs = bflush(ofp) ;

ret0:

#if	CF_DEBUGS
	debugprintf("filewatch_putout: ret rs=%d\n", rs) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filewatch_putout) */


static int filewatch_putoutline(op,ofp,nline,lp,ll)
FILEWATCH	*op ;
bfile		*ofp ;
int		nline ;
const char	*lp ;
int		ll ;
{
	int		rs = SR_OK ;
	int		wlen = 0 ;
	char		cleanbuf[CLEANBUFLEN + 1] ;

	if (hasourbad(lp,ll)) {
	    mkclean(cleanbuf,CLEANBUFLEN,lp,ll) ;
	    lp = (const char *) cleanbuf ;
	}

	    if ((rs >= 0) && op->f.carriage && (lp[0] != '\r')) {
	        rs = bputc(ofp,'\r') ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && (nline > 0) && (op->indent > 0)) {
	        rs = bwriteblanks(ofp,op->indent) ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = bwrite(ofp,lp,ll) ;
	        wlen += rs ;
	    }

	    if ((rs >= 0) && op->f.carriage) {
	        rs = bputc(ofp,'\r') ;
	        wlen += rs ;
	    }

	    if (rs >= 0) {
	        rs = bputc(ofp,'\n') ;
	        wlen += rs ;
	    }

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (filewatch_putoutline) */


static int mkclean(outbuf,outlen,buf,buflen)
char		outbuf[] ;
int		outlen ;
const char	buf[] ;
int		buflen ;
{
	int		i ;
	for (i = 0 ; (i < outlen) && (i < buflen) ; i += 1) {
	    outbuf[i] = buf[i] ;
	    if (isourbad(buf[i] & UCHAR_MAX)) {
	        outbuf[i] = 0xBF ;	/* upside-down question-mark */
	    }
	} /* end for */
	return i ;
}
/* end subroutine (mkclean) */


static int hasourbad(s,slen)
const char	s[] ;
int		slen ;
{
	register int	ch ;
	register int	f = FALSE ;

	while (slen && (s[0] != '\0')) {

	    ch = (s[0] & 0xff) ;
	    f = isourbad(ch) ;
	    if (f) break ;

	    s += 1 ;
	    slen -= 1 ;

	} /* end if */

	return f ;
}
/* end subroutine (hasourbad) */


static int isourbad(int c)
{
	int		f = TRUE ;
	switch (c) {
	case CH_SO:
	case CH_SI:
	case CH_SS2:
	case CH_SS3:
	case '\t':
	    f = FALSE ;
	    break ;
	default:
	    f = (! isprintlatin(c)) ;
	    break ;
	} /* end switch */
	return f ;
}
/* end subroutine (isourbad) */



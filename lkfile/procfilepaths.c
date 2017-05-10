/* procfilepaths */

/* process a paths file */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will read (process) a file that has directory paths in
        it.

        This reads the directory paths in the file and creates a new single
        environment variable named 'PATH'. That environment variable is then
        added to the specified list.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<storebuf.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */

#undef	BUFLEN
#define	BUFLEN		(4 * MAXPATHLEN)

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	PATHBUFLEN	(60 * MAXPATHLEN)


/* external subroutines */

extern int	vstrkeycmp(char **,char **) ;
extern int	strnvaluecmp(char *,char *,int) ;
extern int	pathclean(char *,const char *,int) ;


/* externals variables */


/* forward references */

#if	CF_DEBUGS
static int pathdump(const char *,int) ;
#endif


/* local structures */


/* local variables */

static const uchar	fterms[32] = {
	0x00, 0x00, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int procfilepaths(programroot,fname,lp)
const char	programroot[] ;
const char	fname[] ;
VECSTR		*lp ;
{
	FIELD	fsb ;

	bfile	pfile, *pfp = &pfile ;

	int	rs, rs1 ;
	int	c, len ;
	int	fl, cl ;
	int	psi, pbi ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	pathbuf[PATHBUFLEN + 1], *pp ;
	char	*cp, *fp ;

#if	CF_DEBUGS
	char	outname[MAXPATHLEN + 1] ;
#endif


#if	CF_DEBUGS
	debugprintf("procfilepaths: ent=%s\n",fname) ;

	rs = bopenroot(pfp,programroot,fname,outname,"r",0666) ;

	if (rs < 0) {
	    debugprintf("procfilepaths: bopenroot() rs=%d\n",rs) ;
	    return rs ;
	}

	debugprintf("procfilepaths: file=%s\n",outname) ;
#else
	if ((rs = bopenroot(pfp,programroot,fname,NULL,"r",0666)) < 0)
	    return rs ;
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	debugprintf("procfilepaths: opened\n") ;
#endif

/* get the PATH variable as it exists now, if it exists */

	psi = -1 ;
	pbi = 0 ;
	pathbuf[0] = '\0' ;
	if ((psi = vecstr_finder(lp,"PATH",vstrkeycmp,&pp)) >= 0) {

	    rs = storebuf_buf(pathbuf,PATHBUFLEN,pbi,pp,-1) ;

		pbi += rs ;

#if	CF_DEBUGS
	    debugprintf("procfilepaths: existing=>%s<\n",
	        pathbuf) ;
#endif

	}

	if (rs < 0)
		goto ret1 ;

/* read the file and process any paths that we find */

	c = 0 ;
	while ((rs = breadline(pfp,linebuf,LINEBUFLEN)) > 0) {
	    len = rs ;

	    if (linebuf[len - 1] == '\n') len -= 1 ;
	    linebuf[len] = '\0' ;

#if	CF_DEBUGS
	    debugprintf("procfilepaths: line> %t\n",linebuf,len) ;
#endif

	    cp = linebuf ;
	    cl = len ;
	    while ((cl > 0) && CHAR_ISWHITE(*cp)) {
		cp += 1 ;
		cl -= 1 ;
	    }

	    if ((cp[0] == '\0') || (cp[0] == '#'))
	        continue ;

	    if ((rs = field_start(&fsb,cp,cl)) >= 0) {

		fp = buf ;
	        if ((fl = field_sharg(&fsb,fterms,buf,BUFLEN)) > 0) {

#if	CF_DEBUGS
	    debugprintf("procfilepaths: 1 field> %t\n",fp,fl) ;
#endif

	    while ((fl > 1) && (fp[fl - 1] == '/'))
	        fl -= 1 ;

	    fp[fl] = '\0' ;
	    if (pbi > 0) {

	        if (strnvaluecmp(pathbuf,fp,fl) != 0) {

#if	CF_DEBUGS
	            debugprintf("procfilepaths: adding %t\n",
	                fp,fl) ;
#endif

	            c += 1 ;
	            rs = storebuf_char(pathbuf,PATHBUFLEN,pbi,':') ;

			pbi += rs ;
			if (rs >= 0)
	            rs = storebuf_buf(pathbuf,PATHBUFLEN,pbi,
	                fp,fl) ;

			pbi += rs ;

#if	CF_DEBUGS
	debugprintf("procfilepaths: incremental rs=%d\n",rs) ;
	pathdump(pathbuf,pbi) ;
#endif /* CF_DEBUGS */

	        } /* end if (adding) */

	    } else {

	        rs = storebuf_buf(pathbuf,PATHBUFLEN,pbi,"PATH=",5) ;

		pbi += rs ;
	        c += 1 ;
		if (rs >= 0)
	        rs = storebuf_buf(pathbuf,PATHBUFLEN,pbi,
				fp,fl) ;

		pbi += rs ;

	    }

	    } /* end if (non-zero field) */

	    field_finish(&fsb) ;

	    } /* end if (field) */

		if (rs < 0)
			break ;

	} /* end while (reading lines) */

/* do we have the standard path component? */

	cp = "/usr/bin" ;
	if ((rs >= 0) && (strnvaluecmp(pathbuf,cp,-1) != 0)) {

	    c += 1 ;
	    rs = storebuf_char(pathbuf,PATHBUFLEN,pbi,':') ;

		pbi += rs ;
		if (rs >= 0)
	    rs = storebuf_buf(pathbuf,PATHBUFLEN,pbi,
	        cp,-1) ;

		pbi += rs ;
	}

#if	CF_DEBUGS
	debugprintf("procfilepaths: final rs=%d\n",rs) ;
	pathdump(pathbuf,pbi) ;
#endif /* CF_DEBUGS */

/* add the PATH variable back to the environment */

	if ((rs >= 0) && (c > 0)) {

	    if (psi >= 0)
	        vecstr_del(lp,psi) ;

	    vecstr_add(lp,pathbuf,pbi) ;

	}

ret1:
	bclose(pfp) ;

#if	CF_DEBUGS
	debugprintf("procfilepaths: ret rs=%d c=%u\n",rs,c) ;
#endif

ret0:
	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procfilepaths) */


/* local subroutines */


#if	CF_DEBUGS
static int pathdump(pathbuf,pbi)
const char	pathbuf[] ;
int		pbi ;
	{
	int	rs = SR_OK ;
		int	mlen, rlen = pbi ;
	int	wlen = 0 ;
		const char	*pp = pathbuf ;
		while (rlen > 0) {
			mlen = MIN(rlen,40) ;
			rs = debugprintf("procfilepaths: path| %t\n",
				pp,mlen) ;
			wlen += rs ;
			if (rs < 0) break ;
			pp += mlen ;
			rlen -= mlen ;
		}
	return (rs >= 0) ? wlen : rs ;
	}
/* end subroutine (pathdump) */
#endif /* CF_DEBUGS */



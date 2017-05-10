/* procfileenv */

/* process an environment file */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 2001-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 2001 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will read (process) an environment file and put all of
        the environment variables into the string list (supplied). New
        environment variables just get added to the list. Old environment
        variables already on the list are deleted with a new definition is
        encountered.


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
#include	<char.h>
#include	<localmisc.h>


/* local defines */

#define	BUFLEN		(4 * MAXPATHLEN)

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif


/* external subroutines */

extern int	vstrkeycmp(char **,char **) ;

extern char	*strwcpy(char *,const char *,int) ;


/* externals variables */


/* forward references */


/* local structures */


/* local variables */

static const uchar	fterms[32] = {
	0x00, 0x00, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x20,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int procfileenv(programroot,fname,lp)
char	programroot[] ;
char	fname[] ;
VECSTR	*lp ;
{
	FIELD	fsb ;

	bfile	file, *fp = &file ;

	int	rs ;
	int	c, len, cl ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	buf[BUFLEN + 1], *bp ;
	char	*cp ;

#if	CF_DEBUGS
	char	outname[MAXPATHLEN + 1] ;
#endif


#if	CF_DEBUGS
	debugprintf("procfileenv: ent=%s\n",fname) ;
	if ((rs = bopenroot(fp,programroot,fname,outname,"r",0666)) < 0) {
	    debugprintf("procfileenv: bopen rs=%d\n",rs) ;
	    return rs ;
	}
	debugprintf("procfileenv: outname=%s\n",outname) ;
#else
	if ((rs = bopenroot(fp,programroot,fname,NULL,"r",0666)) < 0)
	    return rs ;
#endif /* CF_DEBUGS */

#if	CF_DEBUGS
	debugprintf("procfileenv: opened\n") ;
#endif

	c = 0 ;
	while ((rs = breadline(fp,linebuf,LINEBUFLEN)) > 0) {
	    len = rs ;

	    if (linebuf[len - 1] == '\n')
	        len -= 1 ;

#if	CF_DEBUGS
	    debugprintf("procfileenv: line> %t\n",linebuf,len) ;
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
		int	fl ;
		char	*fp ;


	    fl = field_get(&fsb,fterms,&fp) ;

	    if (fl > 0) {

#if	CF_DEBUGS
	        debugprintf("procfileenv: 1 field> %t\n",fp,fl) ;
#endif

	        if (strncasecmp(fp,"export",fl) == 0)
	            fl = field_get(&fsb,fterms,&fp) ;

#if	CF_DEBUGS
	        debugprintf("procfileenv: 2 field> %t\n",fp,fl) ;
#endif

	        bp = buf ;
	        bp = strwcpy(bp,fp,MIN(fl,BUFLEN)) ;

	        if (bp < (buf + BUFLEN - 1))
	            *bp++ = '=' ;

	        fp = bp ;
	        fl = field_sharg(&fsb,fterms,bp,(buf + BUFLEN - bp)) ;

#if	CF_DEBUGS
	        debugprintf("procfileenv: 3 field> %t\n",fp,fl) ;
#endif
	        if (fl > 0) {
	            bp += fl ;
	        } else
	            bp -= 1 ;

	        *bp = '\0' ;

	        if ((rs = vecstr_finder(lp,buf,vstrkeycmp,NULL)) >= 0)
	            vecstr_del(lp,rs) ;

	        rs = vecstr_add(lp,buf,(bp - buf)) ;

	        c += 1 ;

	    } /* end if */

	    field_finish(&fsb) ;
	    } /* end if (field) */

	    if (rs < 0) break ;
	} /* end while (reading lines) */

	bclose(fp) ;

#if	CF_DEBUGS
	debugprintf("procfileenv: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procfileenv) */



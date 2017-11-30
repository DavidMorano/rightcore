/* procxpath */

/* process a 'xpath' file */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_PATHCLEAN	1		/* clean up the path */
#define	CF_STAT		0		/* directory needs to be there */


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will read (process) a file that has directory paths in
        it. The directory paths are read in and added (one by one) to the
        specified list.

	Synopsis:

	int procxpath(lp,fname)
	VECSTR		*lp ;
	const char	fname[] ;

	Arguments:

	lp		pointer to VECSTR list
	fname		filename to process

	Returns:

	>=0		count of paths read in
	<0		error


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<bfile.h>
#include	<field.h>
#include	<char.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#ifdef	LINE_MAX
#define	LINEBUFLEN	MAX(LINE_MAX,2048)
#else
#define	LINEBUFLEN	2048
#endif
#endif

#define	PATHBUFLEN	(MAXPATHLEN * 2)


/* external subroutines */

extern int	pathclean(char *,const char *,int) ;


/* externals variables */


/* forward references */


/* local structures */


/* local variables */

static const uchar	fterms[32] = {
	0x00, 0x00, 0x00, 0x00,
	0x08, 0x00, 0x00, 0x04,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
} ;


/* exported subroutines */


int procxpath(VECSTR *lp,cchar *fname)
{

#if	CF_STAT
	struct ustat	sb ;
#endif

	FIELD		fsb ;
	bfile		pathfile, *pfp = &pathfile ;
	int		rs ;
	int		len ;
	int		pl ;
	int		c = 0 ;
	int		f_add ;
	char		pathbuf[PATHBUFLEN + 1] ;
	cchar		*pp ;

	if (lp == NULL) return SR_FAULT ;
	if (fname == NULL) return SR_FAULT ;

	if (fname[0] == '\0') return SR_INVALID ;

	if ((rs = bopen(pfp,fname,"r",0666)) >= 0) {
	    const int	llen = LINEBUFLEN ;
	    int		cl ;
	    cchar	*cp ;
	    char	lbuf[LINEBUFLEN + 1] ;

	    while ((rs = breadline(pfp,lbuf,llen)) > 0) {
	        len = rs ;

	        if (lbuf[len - 1] == '\n') len -= 1 ;
	        lbuf[len] = '\0' ;

#if	CF_DEBUGS
	        debugprintf("procxpath: line> %t\n",lbuf,len) ;
#endif

	        cp = lbuf ;
	        cl = len ;
	        while ((cl > 0) && CHAR_ISWHITE(*cp)) {
	            cp += 1 ;
	            cl -= 1 ;
	        }

	        if ((cp[0] == '\0') || (cp[0] == '#'))
	            continue ;

	        if ((rs = field_start(&fsb,cp,cl)) >= 0) {
	            int		fl ;
		    cchar	*fp ;

	            while ((fl = field_get(&fsb,fterms,&fp)) >= 0) {

#if	CF_DEBUGS
	                debugprintf("procxpath: flen=%d\n",fsb.flen) ;
	                debugprintf("procxpath: 1 field> %t\n",fp,fl) ;
#endif

	                pp = fp ;
	                pl = fl ;
	                if (fl > 0) {

#if	CF_PATHCLEAN
	                    pp = pathbuf ;
	                    pl = pathclean(pathbuf,fp,fl) ;
#else
	                    pp = fp ;
	                    pl = fl ;
	                    while ((pl > 1) && (pp[pl - 1] == '/')) {
	                        pl -= 1 ;
	                    }
	                    pp[pl] = '\0' ;	/* should be able to do this */
#endif /* CF_PATHCLEAN */

#if	CF_STAT && 0
	                    rs = u_stat(pp,&sb) ;

	                    f_add = ((rs >= 0) && S_ISDIR(sb.st_mode)) ;
#else
	                    f_add = (pl > 0) ;
#endif /* CF_STAT */

	                } else {
	                    f_add = ((pl == 0) && (fsb.term == ':')) ;
	                }

	                if (f_add && (vecstr_findn(lp,pp,pl) == SR_NOTFOUND)) {

#if	CF_DEBUGS
	                    debugprintf("procxpath: add=%t\n",pp,pl) ;
#endif

#if	CF_STAT
	                    rs = u_stat(pp,&sb) ;
	                    f_add = ((rs >= 0) && S_ISDIR(sb.st_mode)) ;
#endif

	                    if (f_add) {
	                        c += 1 ;
	                        rs = vecstr_add(lp,pp,pl) ;
	                        if (rs < 0) break ;
	                    }

	                } /* end if (needed to add) */

	                if (fsb.term == '#') break ;
	            } /* end while (reading fields) */

	            field_finish(&fsb) ;
	        } /* end if (field) */

	        if (rs < 0) break ;
	    } /* end while (reading lines) */

	    bclose(pfp) ;
	} /* end if (bfile) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procxpath) */



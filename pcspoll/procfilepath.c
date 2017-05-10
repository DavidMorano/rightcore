/* procfilepath */

/* process a 'path' file */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0
#define	CF_CLEANPATH	1


/* revision history:

	= 1998-09-10, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will read (process) a file that has directory paths in
        it.

        The directory paths are read in and added (one by one) to the specified
        list.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<bfile.h>
#include	<field.h>
#include	<vecstr.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((2 * MAXPATHLEN),2048)
#endif

#define	PATHBUFLEN	MAXPATHLEN


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


int procfilepath(programroot,fname,lp)
const char	programroot[] ;
const char	fname[] ;
VECSTR		*lp ;
{
	struct ustat	sb ;

	FIELD	fsb ;

	bfile	file, *fp = &file ;

	int	rs, n, len ;
	int	flen, pl ;
	int	f_add ;

	char	linebuf[LINEBUFLEN + 1] ;
	char	pathbuf[PATHBUFLEN + 1] ;
	char	*pp ;

#if	CF_DEBUGS
	char	outname[MAXPATHLEN + 1] ;
#endif


#if	CF_DEBUGS
	debugprintf("procfilepath: entered=%s\n",fname) ;
#endif

#if	CF_DEBUGS
	rs = bopenroot(fp,programroot,fname,outname,"r",0666) ;
#else
	rs = bopenroot(fp,programroot,fname,NULL,"r",0666) ;
#endif

	if (rs < 0) {

#if	CF_DEBUGS
	    debugprintf("procfilepath: bopen rs=%d\n",rs) ;
#endif

	    return rs ;
	}

#if	CF_DEBUGS
	debugprintf("procfilepath: file=%s\n",outname) ;
#endif

/* read the file and process any directories that we find */

	n = 0 ;
	while ((rs = breadline(fp,linebuf,LINEBUFLEN)) > 0) {

	    len = rs ;
	    if (linebuf[len - 1] == '\n')
		len -= 1 ;

#if	CF_DEBUGS
	    debugprintf("procfilepath: line> %t\n",linebuf,len) ;
#endif

	    if ((rs = field_start(&fsb,linebuf,len)) >= 0) {

		int	fl ;

		char	*fp ;


	    while ((fl = field_get(&fsb,fterms,&fp)) >= 0) {

#if	CF_DEBUGS
	        debugprintf("procfilepath: 1 field> %t\n",fp,fl) ;
#endif

	        fp[fl] = '\0' ;

	        pp = fp ;
	        pl = fl ;
	        if (fl > 0) {

#if	CF_CLEANPATH
	            pp = pathbuf ;
	            pl = pathclean(pathbuf,fp,fl) ;
#else
	            pp = fp ;
	            pl = fl ;
	            while ((pl > 1) && (pp[pl - 1] == '/'))
	                pl -= 1 ;
#endif

	            rs = u_stat(pp,&sb) ;

	            f_add = ((rs >= 0) && S_ISDIR(sb.st_mode)) ;

	        } else
	            f_add = ((pl == 0) && (fsb.term == ':')) ;

	        if (f_add) {

	            if (vecstr_findn(lp,pp,pl) < 0)
	                vecstr_add(lp,pp,pl) ;

	        } /* end if (is a directory) */

	        if (fsb.term == '#')
	            break ;

	    } /* end while (reading fields) */

	    field_finish(&fsb) ;

	    } /* end if */

	} /* end while (reading lines) */

	bclose(fp) ;

#if	CF_DEBUGS
	debugprintf("procfilepath: ret rs=%d n=%u\n",rs,n) ;
#endif

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (procfilepath) */




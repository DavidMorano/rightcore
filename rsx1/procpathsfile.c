/* procfilepaths */

/* process a paths file */
/* version %I% last modified %G% */


#define	F_DEBUGS	0


/* revision history :

	= 94/09/10, David A­D­ Morano

	This program was originally written.


*/



/*****************************************************************************

	This subroutine will read (process) a file that has
	directory paths in it.

	This reads the directory paths in the file and creates
	a new single environment variable named 'PATH'.
	That environment variable is then added to the specified list.



*****************************************************************************/



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

#include	"misc.h"



/* local defines */

#undef	BUFLEN
#define	BUFLEN		(4 * MAXPATHLEN)
#undef	LINELEN
#define	LINELEN		(2 * MAXPATHLEN)
#define	PATHBUFLEN	(30 * MAXPATHLEN)



/* external subroutines */

extern int	vstrkeycmp(char **,char **) ;
extern int	strnvaluecmp(char *,char *,int) ;


/* externals variables */


/* forward references */


/* local global variabes */


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






int procfilepaths(programroot,fname,lp)
char	programroot[] ;
char	fname[] ;
VECSTR	*lp ;
{
	bfile	file, *fp = &file ;

	FIELD	fsb ;

	int	len, i, rs ;
	int	pathbuflen, psi, pbi ;

	char	linebuf[LINELEN + 1] ;
	char	buf[BUFLEN + 1] ;
	char	pathbuf[PATHBUFLEN + 1], *pp ;
	char	*cp ;

#if	F_DEBUGS
	char	outname[MAXPATHLEN + 1] ;
#endif


#if	F_DEBUGS
	eprintf("procfilepaths: entered=%s\n",fname) ;

	if ((rs = bopenroot(fp,programroot,fname,outname,"r",0666)) < 0) {

	    eprintf("procfilepaths: bopen rs=%d\n",rs) ;

	    return rs ;
	}

	eprintf("procfilepaths: file=%s\n",outname) ;
#else
	if ((rs = bopenroot(fp,programroot,fname,NULL,"r",0666)) < 0)
	    return rs ;
#endif

#if	F_DEBUGS
	eprintf("procfilepaths: opened\n") ;
#endif

/* get the PATH variable as it exists now, if it exists */

	psi = -1 ;
	pbi = 0 ;
	pathbuf[0] = '\0' ;
	if ((rs = vecstr_finder(lp,"PATH",vstrkeycmp,&pp)) >= 0) {

	    psi = rs ;
	    pbi += storebuf_buf(pathbuf,PATHBUFLEN,pbi,pp,-1) ;

#if	F_DEBUGS
	    eprintf("procfilepaths: existing %s\n",
	        pathbuf) ;
#endif

	}


/* read the file and process any paths that we find */

	i = 0 ;
	while ((len = bgetline(fp,linebuf,LINELEN)) > 0) {

#if	F_DEBUGS
	    eprintf("procfilepaths: line> %W\n",linebuf,len - 1) ;
#endif

	    fsb.lp = linebuf ;
	    fsb.rlen = (linebuf[len - 1] == '\n') ? (len - 1) : len ;
	    field_sharg(&fsb,fterms,buf,BUFLEN) ;

	    if (fsb.flen < 0) continue ;

	    if ((fsb.flen == 0) && (fsb.term == '#')) continue ;

#if	F_DEBUGS
	    eprintf("procfilepaths: flen=%d\n",fsb.flen) ;
#endif

#if	F_DEBUGS
	    eprintf("procfilepaths: 1 field> %W\n",fsb.fp,fsb.flen) ;
#endif

	    while ((fsb.flen > 1) && (fsb.fp[fsb.flen - 1] == '/'))
	        fsb.flen -= 1 ;

	    fsb.fp[fsb.flen] = '\0' ;
	    if (pbi > 0) {

	        if (strnvaluecmp(pathbuf,fsb.fp,fsb.flen) != 0) {

#if	F_DEBUGS
	            eprintf("procfilepaths: adding %W\n",
	                fsb.fp,fsb.flen) ;
#endif

	            i += 1 ;
	            pbi += storebuf_char(pathbuf,PATHBUFLEN,pbi,':') ;

	            pbi += storebuf_buf(pathbuf,PATHBUFLEN,pbi,
	                fsb.fp,fsb.flen) ;

	        }

	    } else {

	        pbi += storebuf_buf(pathbuf,PATHBUFLEN,pbi,"PATH=",5) ;

	        i += 1 ;
	        pbi += storebuf_buf(pathbuf,PATHBUFLEN,pbi,fsb.fp,fsb.flen) ;

	    }

	} /* end while */

/* do we have the standard path component ? */

	cp = "/bin" ;
	if (strnvaluecmp(pathbuf,cp,-1) != 0) {

	    i += 1 ;
	    pbi += storebuf_char(pathbuf,PATHBUFLEN,pbi,':') ;

	    pbi += storebuf_buf(pathbuf,PATHBUFLEN,pbi,
	        cp,-1) ;

	}

/* add the PATH variable back to the environment */

	if (i > 0) {

	    if (psi >= 0)
	        vecstr_del(lp,psi) ;

	    vecstr_add(lp,pathbuf,pbi) ;

	}

	bclose(fp) ;

#if	F_DEBUGS
	eprintf("procfilepaths: exiting rs=%d\n",
	    ((len < 0) ? len : i)) ;
#endif

	return ((len < 0) ? len : i) ;
}
/* end subroutine (procfilepaths) */




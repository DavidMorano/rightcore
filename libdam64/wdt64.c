/* wdt */

/* walk directory tree */


#define	CF_DEBUGS	0
#define	CF_ACCESS	0

#ifndef	CF_LF64
#define	CF_LF64		0
#endif


/* revision history:

	= 1991-05-17, David A­D­ Morano

	This was made to get around some 'ftw(3c)' limitations.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/***********************************************************************

	This subroutine is similar to the 'ftw()' subroutine in 
	that it "walks" a directory hierarchy.

	Note that unlike 'ftw()' this subroutine is NOT recursive !


	Synopsis:

	int wdt(basedir,mode,uf,uarg)
	char	basedir[] ;
	int	mode ;
	int	(*uf)() ;
	void	*uarg ;

	Arguments:

	- basedir	directory at top of tree
	- mode		mode of usuage (FOLLOW links or not)
	- uf		user function to call
	- uarg		user argument (usually a pointer)

	Returns:

	<0		error
	>=0		OK


***********************************************************************/


#define	WDT_MASTER	1


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include	<dirent.h>

#include	<bfile.h>
#include	<fsdir.h>
#include	<localmisc.h>

#include	"wdt.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN		((2*MAXPATHLEN)+10)
#endif

#ifndef	TMPDNAME
#define	TMPDNAME	"/tmp"
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;


/* external variables */


/* exported subroutines */


int wdt(basedir,mode,uf,uarg)
const char	basedir[] ;
int		mode ;
int		(*uf)() ;
void		*uarg ;
{
	struct dirent	*dep ;

#if	CF_LF64
	struct stat64	se ;
#else
	struct ustat	se ;
#endif

	fsdir		dir ;

	fsdir_ent	ds ;

	bfile		namefile, *nfp = &namefile ;
	bfile		nnfile, *nnfp = &nnfile ;

	offset_t	offset ;

	int	rs, blen, len, slen, dlen, wlen, l ;
	int	ndir = 0 ;
	int	rs_stat ;
	int	f_seed = TRUE ;
	int	f_breakout = FALSE ;

	char	nfname[MAXPATHLEN + 1] ;
	char	dirbuf[(MAXPATHLEN * 2) + 1] ;
	char	*tmpdname ;
	char	*linebuf, *lp ;
	char	*bdp, *dnp ;
	char	*dirnamep ;


#if	CF_DEBUGS
	if (basedir != NULL)
	    debugprintf("wdt: entered\n") ;
#endif

	if ((tmpdname = getenv("TMPDIR")) == NULL)
	    tmpdname = TMPDNAME ;

	nfname[0] = '\0' ;

#if	CF_DEBUGS
	if (basedir != NULL)
	    debugprintf("wdt: basedir=\"%s\"\n",basedir) ;
#endif

	nfname[0] = '\0' ;
	bdp = (char *) basedir ;
	if ((bdp == NULL) || (strcmp(bdp,".") == 0))
	    bdp = "" ;

	mkpath2(dirbuf,tmpdname, "dirnamXXXXXXXX") ;

	rs = mktmpfile(nfname,0600,dirbuf) ;

	if (rs < 0)
		goto ret0 ;

	rs = bopen(nfp,nfname,"rwct",0600) ;

	if (rs < 0)
		goto ret1 ;

	rs = bopen(nnfp,nfname,"wct",0666) ;

	if (rs < 0)
		goto ret2 ;

/* set line buffering on the file so that we do not stall reading it */

	rs = bcontrol(nnfp,BC_LINEBUF,0) ;

	if (rs < 0)
		goto ret3 ;

	u_unlink(nfname) ;

	nfname[0] = '\0' ;

/* preload the base directory name into the working directory name buffer */

	blen = strlen(bdp) ;

	if (blen > 0) {

	    strcpy(dirbuf,bdp) ;

	    dirbuf[blen++] = '/' ;

	}

	linebuf = dirbuf + blen ;

/* go through the loops */

#if	CF_DEBUGS
	debugprintf("wdt: before while >%s<\n",bdp) ;
#endif

	len = 0 ;
	offset = 0 ;
	while (f_seed || ((len = breadline(nfp,linebuf,LINEBUFLEN)) > 0)) {

	    if (len && (linebuf[len - 1] == '\n')) 
		len -= 1 ;

	    linebuf[len] = '\0' ;
	    f_seed = FALSE ;

#if	CF_DEBUGS
	    if (len)
	        debugprintf("wdt: line >%t<\n",linebuf,len) ;

	    else
	        debugprintf("wdt: dirbuf >%t<\n",dirbuf,blen) ;
#endif /* CF_DEBUGS */

	    if (blen == 0)
	        dirnamep = "." ;

	    else
	        dirnamep = dirbuf ;

	    if (fsdir_open(&dir,dirnamep) < 0) 
		continue ;

	    while ((dlen = fsdir_read(&dir,&ds)) > 0) {

	        dep = &ds.entry ;
	        dnp = dep->d_name ;

#if	CF_DEBUGS
	        debugprintf("wdt: top of inner while > %s\n",dep->d_name) ;
#endif

/* do not play with the "special" entries ! (this is a fast way of deciding) */

	        if (*dnp == '.') {

	            if (dnp[1] == '\0') 
			continue ;

	            if ((dnp[1] == '.') && (dnp[2] == '\0'))
	                continue ;

	        }

#if	CF_DEBUGS
	        debugprintf("wdt: 1 con\n") ;
#endif

/* PROGRAM NOTE :

	At this point :
	= blen		base length (inluding a trailing slash character)
	= len		length of current directory from base
	= slen		will account for the slash character if needed

*/

/* prepare to 'stat' this entry */

	        slen = 0 ;
	        if (len > 0) {

	            slen = 1 ;
	            linebuf[len] = '/' ;

	        }

	        strcpy(linebuf + len + slen,dep->d_name) ;


/* can we 'stat' the thing ? */

#if	CF_LF64
	        rs_stat = u_lstat64(dirbuf,&se) ;
#else
	        rs_stat = u_lstat(dirbuf,&se) ;
#endif

	        if (rs_stat < 0) {

	            se.st_mode = 0 ;
	            se.st_dev = 0 ;
	            se.st_ino = 0 ;
	            se.st_atime = 0 ;
	            se.st_mtime = 0 ;
	            se.st_ctime = 0 ;

	        }


/* call the user's supplied function, if present */

/*
	We call the user's function and look at his returned value.
	We act on the returned value as follows :

	=0	continue on to next node
	>0	skip this entry and go on to the next
	<0	quit the directory tree enumeration

*/

	        if (uf != NULL) {

#if	CF_DEBUGS
	            debugprintf("wdt: calling user function\n") ;
#endif

	            rs = (*uf)(dirbuf,&se,uarg) ;

#if	CF_DEBUGS
	            debugprintf("wdt: back from user, rs=%d\n",rs) ;
#endif

	            if (rs > 0) 
			continue ;

	            if (rs < 0) {
			f_breakout = TRUE ;
			break ;
		    }

	        } /* end if (calling user's function) */

#if	CF_DEBUGS
	        debugprintf("wdt: 3 con\n") ;
#endif

/* from now on, we only want to deal with directories that we find */

	        if (rs_stat < 0) 
			continue ;

	        if (S_ISLNK(se.st_mode) && (mode & WDT_MFOLLOW)) {

#if	CF_LF64
	            rs_stat = u_stat64(dirbuf,&se) ;
#else
	            rs_stat = u_stat(dirbuf,&se) ;
#endif

	        if (rs_stat < 0) 
			continue ;

		}

	        if (! S_ISDIR(se.st_mode)) 
			continue ;

#if	CF_DEBUGS
	        debugprintf("wdt: about to process a directory\n") ;
#endif

/* see if the real user can access the directory */

#if	CF_ACCESS
	        if (u_access(dirbuf,R_OK) < 0) 
			continue ;
#endif

/* OK, we are free, prepare to write out information on this directory */

	        ndir += 1 ;

/* write out the directory name as a coded line */

#if	CF_DEBUGS
	        debugprintf("wdt: writing a name\n%s\n",linebuf) ;
#endif

	        wlen = len + slen + dlen + 1 ;
	        rs = bprintf(nnfp,"%t\n",linebuf,(wlen - 1)) ;

		if (rs < wlen) {

#if	CF_DEBUGS
	            debugprintf("wdt: bad write (rs %d)\n",rs) ;
#endif
	            rs = SR_DQUOT ;
			f_breakout = TRUE ;
	            break ;
	        }

	        offset += wlen ;

	    } /* end while (inner) */

	    fsdir_close(&dir) ;

		if (f_breakout)
			break ;

	} /* end while (outer) */


/* clean up on a good exit */
ret3:
	bclose(nnfp) ;

ret2:
	bclose(nfp) ;

ret1:
	if ((nfname != NULL) && (nfname[0] != '\0'))
	    u_unlink(nfname) ;

ret0:
	return (rs >= 0) ? ndir : rs ;
}
/* end subroutine (wdt) */




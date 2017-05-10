/* walk */

/* walk directory tree in a custom way for COPYCD */


#define	CF_DEBUGS	0


/***********************************************************************

	This subroutine is similar to the 'ftw' subroutine in 
	that it "walks" a directory hierarchy.

	Note that unlike 'ftw' this subroutine is NOT recursive !

	Arguments:
	- basedir	directory at top of tree
	- mode		mode of usuage (FOLLOW links or not)
	- uf		user function to call
	- uarg		user argument (usually a pointer)


***********************************************************************/


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
#define	LINEBUFLEN		(MAXPATHLEN + 10)
#endif


/* external subroutines */


/* external variables */


/* exported subroutines */


int walk(pip,srcdir,dstdir,uf,uarg)
struct proginfo	*pip ;
char	basedir[] ;
int	(*uf)() ;
void	*uarg ;
{
	struct ustat	se ;

	fsdir		dir ;
	fsdir_ent	ds ;

	struct dirent	*dep ;

	int	rs, blen, len, slen, dlen, wlen, l ;
	int	ndir = 0 ;
	int	f_seed = TRUE ;
	int	rs_stat ;

	char	nfname[MAXPATHLEN + 1] ;
	char	*linebuf, *lp ;
	char	dirbuf[(MAXPATHLEN * 2) + 1] ;
	char	*bdp, *dnp ;
	char	*tmpdir ;
	char	*dirnamep ;


#if	CF_DEBUGS
	if (basedir != NULL)
	    debugprintf("walk: entered\n") ;
#endif

	nfname[0] = '\0' ;
#if	CF_DEBUGS
	if (basedir != NULL)
	    debugprintf("walk: basedir=\"%s\"\n",basedir) ;
#endif

	nfname[0] = '\0' ;
	bdp = basedir ;
	if ((bdp == NULL) || (strcmp(bdp,".") == 0))
	    bdp = "" ;

/* preload the base directory name into the working directory name buffer */

	blen = strlen(bdp) ;

	if (blen > 0) {

	    strcpy(dirbuf,bdp) ;

	    dirbuf[blen++] = '/' ;

	}

	linebuf = dirbuf + blen ;

/* go through the loops */

#if	CF_DEBUGS
	debugprintf("walk: before while >%s<\n",bdp) ;
#endif

	    if ((rs = fsdir_open(&dir,dirnamep)) >= 0) {

	    while ((dlen = fsdir_read(&dir,&ds)) > 0) {

	        dep = &ds.entry ;
	        dnp = dep->d_name ;

#if	CF_DEBUGS
	        debugprintf("walk: top of inner while > %s\n",dep->d_name) ;
#endif

/* do not play with the "special" entries ! (this is a fast way of deciding) */

	        if (*dnp == '.') {

	            if (dnp[1] == '\0') continue ;

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

	        rs_stat = u_lstat(dirbuf,&se) ;

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

	            if (rs > 0) continue ;

	            if (rs < 0) goto breakout ;

	        } /* end if (calling user's function) */

#if	CF_DEBUGS
	        debugprintf("wdt: 3 con\n") ;
#endif

/* from now on, we only want to deal with directories that we find */

	        if (rs_stat < 0) continue ;

	        if (mode & WALKM_FOLLOW)
	            rs_stat = u_stat(dirbuf,&se) ;

	        if (! S_ISDIR(se.st_mode)) continue ;

#if	CF_DEBUGS
	        debugprintf("wdt: about to process a directory\n") ;
#endif

/* see if the real user can access the directory */

	        if (u_access(dirbuf,R_OK) < 0) continue ;

/* OK, we are free, prepare to write out information on this directory */

	        ndir += 1 ;

/* write out the directory name as a coded line */

#if	CF_DEBUGS
	        debugprintf("wdt: writing a name\n%s\n",linebuf) ;
#endif

	        wlen = len + slen + dlen + 1 ;
	        if ((rs = bprintf(nnfp,"%W\n",linebuf,wlen - 1)) < wlen) {

#if	CF_DEBUGS
	            debugprintf("wdt: bad write (rs %d)\n",rs) ;
#endif
	            rs = WALKR_BADWRITE ;
	            goto bad5 ;
	        }

	        offset += wlen ;

	    } /* end while (inner) */

	    fsdir_close(&dir) ;

	} /* end if */


/* clean up on a good exit */
exit:

	return ndir ;

breakout:

	goto exit ;

/* handle the bad errors and associated cleanup */
bad5:
	fsdir_close(&dir) ;

bad3:

bad2:

bad1:

badret:
	return rs ;
}
/* end subroutine (walk) */




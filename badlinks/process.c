/* process */

/* process the directory tree */


#define	F_DEBUG 	0		/* switchable debug print-outs */
#define	F_REMOVE 	1


/* revision history :

	= 1996-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/***********************************************************************

	Arguments:
	- pip		program information pointer
	- basedir	directory at top of tree


***********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<dirent.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<sfstr.h>
#include	<bfile.h>
#include	<fsdir.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"process.h"


/* local defines */

#ifndef	LINELEN
#define	LINELEN		(MAXPATHLEN + 10)
#endif


/* external subroutines */

extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	checklink(struct proginfo *,char *,struct ustat *) ;


/* external variables */


/* external subroutines */


int process(pip,basedir)
struct proginfo	*pip ;
char		basedir[] ;
{
	bfile		namefile, *nfp = &namefile ;
	bfile		nnfile, *nnfp = &nnfile ;

	FSDIR		dir ;
	FSDIR_SLOT	ds ;

	struct dirent	*dep ;

	struct ustat	se ;

	int	rs ;
	int	blen, len, slen, dlen, wlen ;
	int	realdirlen ;
	int	capbaselen ;
	int	ndir = 0 ;
	int	ndel = 0 ;
	int	rs_stat ;
	int	f_seed = TRUE ;
	int	f_cap ;

	char	nfname[MAXPATHLEN + 1] ;
	char	*linebuf ;
	char	dirbuf[(MAXPATHLEN * 2) + 1] ;
	char	realdirbuf[(MAXPATHLEN * 2) + 1] ;
#if	F_DEBUG
	char	debugbuf[(MAXPATHLEN * 2) + 1] ;
#endif
	char	*bdp, *dnp ;
	char	*capbasep ;
	char	*dirnamep ;
	char	*cp ;


	if (u_access(basedir,R_OK) != 0) {

	    bprintf(pip->efp,
	        "%s: could not access directory \"%s\"\n",
	        pip->progname,basedir) ;

	    return BAD ;
	}


	if (pip->f.verbose)
	    bprintf(pip->ofp,"directory=%s\n",basedir) ;


	nfname[0] = '\0' ;
	bdp = basedir ;
	if ((bdp == NULL) || (strcmp(bdp,".") == 0)) bdp = "" ;

	bufprintf(dirbuf,MAXPATHLEN,"%s/dirnamXXXXXXXX",pip->tmpdir) ;

	if ((rs = mktmpfile(nfname,0600,dirbuf)) < 0)
	    return PROR_BADTMP ;

	if ((rs = bopen(nfp,nfname,"rwct",0600)) < 0) {

	    rs = PROR_BADTMPOPEN ;
	    goto bad1 ;
	}

	if ((rs = bopen(nnfp,nfname,"wct",0666)) < 0) {

	    rs = PROR_BADTMPOPEN ;
	    goto bad2 ;
	}

/* set line buffering on the file so that we do not stall reading it */

	if ((rs = bcontrol(nnfp,BC_LINEBUF,0)) < 0) {

	    rs = PROR_BADTMPOPEN ;
	    goto bad3 ;
	}

/* remove the temporary file now (for safer cleanup) */

	unlink(nfname) ;

/* preload the base directory name into the working directory name buffer */

	blen = strlen(bdp) ;

	if (blen > 0) {

	    strcpy(dirbuf,bdp) ;

	    strcat(dirbuf,"/") ;

	    blen += 1 ;

	}

	linebuf = dirbuf + blen ;

#if	F_DEBUG
	if (pip->debuglevel > 1) 
		eprintf("process: starting dirbuf=%s dirlen=%d\n",
	    dirbuf,blen) ;
#endif

/* go through the loops */

	len = 0 ;
	while (f_seed || ((len = bgetline(nfp,linebuf,LINELEN)) > 0)) {

	    if (len && (linebuf[len - 1] == '\n')) len -= 1 ;

	    linebuf[len] = '\0' ;
	    f_seed = FALSE ;

#if	F_DEBUG
	    if (pip->debuglevel > 1)
	        eprintf("process: top outer while >%W<\n",
	            linebuf,len) ;
#endif

#if	F_DEBUG
	    if (pip->debuglevel > 1) eprintf(
	        "process: outer loop dirbuf=%s len=%d\n",
	        dirbuf,len) ;
#endif

/* if we cannot open the directory, it can't be too important to us */

	    if (blen == 0) {
	        dirnamep = "." ;
	    } else
	        dirnamep = dirbuf ;

	    if (fsdir_open(&dir,dirnamep) < 0) 
		continue ;

#if	F_DEBUG
	    if (pip->debuglevel > 1) 
		eprintf("process: directory \"%s\"\n",
	        dirbuf) ;
#endif

/* rock and roll ! */

	    while ((dlen = fsdir_read(&dir,&ds)) > 0) {

	        dep = &ds.entry ;
	        dnp = dep->d_name ;

#if	F_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("process: top of inner while name=%s\n",
	                dnp) ;
#endif

/* do not play with the "special" entries ! (this is a fast way of deciding) */

	        if (*dnp == '.') {

	            if (dnp[1] == '\0') continue ;

	            if ((dnp[1] == '.') && (dnp[2] == '\0'))
	                continue ;

	        }

#if	F_DEBUG
	        if (pip->debuglevel > 2)
	            eprintf("process: continuing (inner) w/ name=%s\n",
	                dnp) ;
#endif

/* PROGRAM NOTE :
		
	At this point :
	= blen		base length (inluding a trailing slash character)
	= len		length of current directory from base
	= slen		will account for the slash character if needed

*/


/* prepare to 'stat' this entry */

#if	F_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("process: regular processing\n") ;
#endif

	        slen = 0 ;
	        if (len > 0) {

	            slen = 1 ;
	            strcpy(linebuf + len,"/") ;

	        }

	        strcpy(linebuf + len + slen,dnp) ;


	        if ((rs_stat = u_lstat(dirbuf,&se)) < 0) {

	            se.st_mode = 0 ;
	            se.st_dev = 0 ;
	            se.st_ino = 0 ;
	            se.st_atime = 0 ;
	            se.st_mtime = 0 ;
	            se.st_ctime = 0 ;

	        }


/* play with it ! */

	        if (! S_ISDIR(se.st_mode)) {

	            if ((rs = checklink(pip,dirbuf,&se)) > 0) 
			continue ;

	            if (rs < 0) 
			goto breakout ;

	        } /* end if (calling user evaluate function) */


/* from now on, we only want to deal with directories that we find */

	        if ((rs_stat >= 0) && (S_ISLNK(se.st_mode)))
	            rs_stat = u_stat(dirbuf,&se) ;

	        if (rs_stat < 0) continue ;

	        if (! S_ISDIR(se.st_mode)) continue ;

#if	F_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("process: about to process a directory\n") ;
#endif

/* see if the real user can access the directory */

	        if (u_access(dirbuf,R_OK) < 0) continue ;

/* OK, we are free, prepare to write out information on this directory */

	        ndir += 1 ;

/* write out the directory name as a coded line */

#if	F_DEBUG
	        if (pip->debuglevel > 1)
	            eprintf("process: writing a name\n%s\n",
	                linebuf) ;
#endif

	        wlen = len + slen + dlen + 1 ;
	        if ((rs = bprintf(nnfp,"%W\n",linebuf,wlen - 1)) < wlen) {

#if	F_DEBUG
	            if (pip->debuglevel > 1)
	                eprintf("process: bad write (rs %d)\n",
	                    rs) ;
#endif
	            goto bad5 ;
	        }

	    } /* end while (inner) */

	    fsdir_close(&dir) ;

	} /* end while (outer) */


/* clean up on a good exit */
exit:
	bclose(nfp) ;

	bclose(nnfp) ;

	if (pip->f.verbose && (ndel > 0)) {

	    bprintf(pip->ofp,"subdirectories scanned: %d\n",
	        ndir) ;

	    bprintf(pip->ofp,"number files deleted:   %d\n",
	        ndel) ;

	}

	return ndir ;

breakout:
	fsdir_close(&dir) ;

	goto exit ;

/* handle the bad errors and associated cleanup */
bad5:
	fsdir_close(&dir) ;

bad3:
	bclose(nnfp) ;

bad2:
	bclose(nfp) ;

bad1:
	if ((nfname != NULL) && (nfname[0] != '\0'))
	    unlink(nfname) ;

badret:
	return rs ;
}
/* end subroutine (process) */




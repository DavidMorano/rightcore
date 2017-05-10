/* procfile */

/* process the directory tree */


#define	CF_DEBUG 	0		/* switchable debug print-outs */
#define	CF_REMOVE 	1


/* revision history:

	= 1996-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/***********************************************************************

	Synopsis:

	int procfile(pip,basedname)
	struct proginfo	*pip ;
	const char	basedname[] ;

	Arguments:

	- pip		program information pointer
	- basedname	directory at top of tree

	Returns:

	<0		error
	>=0		OK


***********************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<fsdir.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"
#include	"procfile.h"


/* local defines */

#ifndef	LINEBUFLEN
#define	LINEBUFLEN	MAX((MAXPATHLEN + 10),2048)
#endif


/* external subroutines */

extern int	mktmpfile(char *,mode_t,const char *) ;
extern int	checklink(struct proginfo *,const char *,struct ustat *) ;


/* external variables */


/* exported subroutines */


int procfile(pip,basedname)
struct proginfo	*pip ;
const char	basedname[] ;
{
	struct ustat	se ;

	FSDIR		dir ;

	FSDIR_ENT	ds ;

	bfile		namefile, *nfp = &namefile ;
	bfile		nnfile, *nnfp = &nnfile ;

	int	rs, blen, len, slen, dlen, wlen ;
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
#if	CF_DEBUG
	char	debugbuf[(MAXPATHLEN * 2) + 1] ;
#endif
	char	*bdp, *dnp ;
	char	*capbasep ;
	char	*dirnamep ;
	char	*cp ;


	if (basedname == NULL)
		return SR_FAULT ;

	if (basedname[0] == '\0')
		return SR_INVALID ;

	rs = u_access(basedname,R_OK) ;

	if (rs < 0) {

	    bprintf(pip->efp,
	        "%s: could not access directory=%s (%d)\n",
	        pip->progname,basedname,rs) ;

	    goto ret0 ;
	}


	if (pip->verboselevel > 0)
	    bprintf(pip->ofp,"directory=%s\n",basedname) ;


	nfname[0] = '\0' ;
	bdp = (char *) basedname ;
	if ((bdp == NULL) || (strcmp(bdp,".") == 0)) 
		bdp = "" ;

	mkpath2(dirbuf, pip->tmpdname, "dirnamXXXXXXXX") ;

	rs = mktmpfile(nfname,0600,dirbuf) ;

	if (rs < 0)
		goto ret0 ;

	rs = bopen(nfp,nfname,"rwct",0600) ;

	if (rs < 0)
		goto bad1 ;

	rs = bopen(nnfp,nfname,"wct",0666) ;

	if (rs < 0)
	    goto bad2 ;

/* set line buffering on the file so that we do not stall reading it */

	rs = bcontrol(nnfp,BC_LINEBUF,0) ;

	if (rs < 0)
	    goto bad3 ;

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

#if	CF_DEBUG
	if (pip->debuglevel > 1) 
		debugprintf("procfile: starting dirbuf=%s dirlen=%d\n",
	    dirbuf,blen) ;
#endif

/* go through the loops */

	len = 0 ;
	while (f_seed || ((len = breadline(nfp,linebuf,LINEBUFLEN)) > 0)) {

	    if (len && (linebuf[len - 1] == '\n')) len -= 1 ;

	    linebuf[len] = '\0' ;
	    f_seed = FALSE ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("procfile: top outer while >%t<\n",
	            linebuf,len) ;
#endif

#if	CF_DEBUG
	    if (pip->debuglevel > 1) debugprintf(
	        "procfile: outer loop dirbuf=%s len=%d\n",
	        dirbuf,len) ;
#endif

/* if we cannot open the directory, it can't be too important to us */

	    if (blen == 0)
	        dirnamep = "." ;

	    else
	        dirnamep = dirbuf ;

	    if (fsdir_open(&dir,dirnamep) < 0) 
		continue ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) 
		debugprintf("procfile: directory \"%s\"\n",
	        dirbuf) ;
#endif

/* rock and roll ! */

	    while ((dlen = fsdir_read(&dir,&ds)) > 0) {

	        dnp = ds.name ;
	        if (*dnp == '.') {

	            if (dnp[1] == '\0') 
			continue ;

	            if ((dnp[1] == '.') && (dnp[2] == '\0'))
	                continue ;

	        }

#if	CF_DEBUG
	        if (pip->debuglevel > 2)
	            debugprintf("procfile: continuing (inner) w/ name=%s\n",
	                dnp) ;
#endif

/* PROGRAM NOTE :
		
	At this point :
	= blen		base length (inluding a trailing slash character)
	= len		length of current directory from base
	= slen		will account for the slash character if needed

*/


/* prepare to 'stat' this entry */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("procfile: regular processing\n") ;
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
			break ;

	        } /* end if (calling user evaluate function) */

/* from now on, we only want to deal with directories that we find */

	        if ((rs_stat >= 0) && (S_ISLNK(se.st_mode)))
	            rs_stat = u_stat(dirbuf,&se) ;

	        if (rs_stat < 0) 
			continue ;

	        if (! S_ISDIR(se.st_mode)) 
			continue ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("procfile: about to process a directory\n") ;
#endif

/* see if the real user can access the directory */

	        if (u_access(dirbuf,R_OK) < 0) 
			continue ;

/* OK, we are free, prepare to write out information on this directory */

	        ndir += 1 ;

/* write out the directory name as a coded line */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("procfile: writing a name\n%s\n",
	                linebuf) ;
#endif

	        wlen = len + slen + dlen + 1 ;
	        rs = bprintf(nnfp,"%t\n",linebuf,(wlen - 1)) ;

		if (rs < 0)
			break ;

	    } /* end while (inner) */

	    fsdir_close(&dir) ;

	    if (rs < 0)
		break ;

	} /* end while (outer) */

/* clean up on a good exit */
exit:
	bclose(nfp) ;

	bclose(nnfp) ;

	if ((pip->verboselevel > 0) && (ndel > 0)) {

	    bprintf(pip->ofp,"subdirectories scanned: %d\n",
	        ndir) ;

	    bprintf(pip->ofp,"number files deleted:   %d\n",
	        ndel) ;

	}

ret0:
	return (rs >= 0) ? ndir : rs ;

/* handle the bad errors and associated cleanup */
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
/* end subroutine (procfile) */




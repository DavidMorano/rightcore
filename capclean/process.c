/* process */

/* process the directory tree */


#define	CF_DEBUG 	0
#define	CF_REMOVE 	1


/* revision history:

	= 1996-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/***********************************************************************

	Arguments:

	- basedir	directory at top of tree


***********************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/mman.h>
#include	<unistd.h>
#include	<string.h>
#include	<dirent.h>
#include	<limits.h>

#include	<vsystem.h>
#include	<sfstr.h>
#include	<bfile.h>
#include	<fsdir.h>
#include	<localmisc.h>

#include	"defs.h"
#include	"pror.h"


/* local routine defines */

#define	LINEBUFLEN		(MAXPATHLEN + 10)


/* external subroutines */

extern int	mktmpfile(char *,mode_t,const char *) ;


/* external variables */


/* exported subroutines */


int process(pip,basedir)
struct proginfo	*pip ;
const char	basedir[] ;
{
	struct dirent	*dep ;

	struct ustat	se ;

	FSDIR		dir ;
	FSDIR_ENT	ds ;

	bfile		namefile, *nfp = &namefile ;
	bfile		nnfile, *nnfp = &nnfile ;

	const int	llen = LINEBUFLEN ;

	int	rs ;
	int	blen, len, slen, dlen, wlen ;
	int	realdirlen ;
	int	capbaselen ;
	int	ndir = 0 ;
	int	ndel = 0 ;
	int	f_seed = TRUE ;
	int	f_cap ;

	const char	*cp ;

	char	*lbuf ;
	char	nfname[MAXPATHLEN + 1] ;
	char	dirbuf[(MAXPATHLEN * 2) + 1] ;
	char	realdirbuf[(MAXPATHLEN * 2) + 1] ;
	char	debugbuf[(MAXPATHLEN * 2) + 1] ;
	char	debuf[sizeof(struct dirent) + _POSIX_PATH_MAX + 1] ;
	char	*bdp, *dnp ;
	char	*capbasep ;
	char	*dirnamep ;


#if	CF_DEBUG && 0
	len = sfdirname(basedir,-1,&cp) ;

	debugprintf("process: basedir=\"%t\"\n",cp,len) ;

	if (basedir != NULL) 
		return 0 ;
#endif

	if ((rs = u_access(basedir,R_OK)) < 0) {

	    bprintf(pip->efp,
	        "%s: could not access directory \"%s\"\n",
	        pip->progname,basedir) ;

	    return rs ;
	}


	if (pip->f.verbose)
	    bprintf(pip->ofp,"directory: %s\n",basedir) ;


	nfname[0] = '\0' ;
	bdp = basedir ;
	if ((bdp == NULL) || (strcmp(bdp,".") == 0)) 
		bdp = "" ;

	if ((rs = mktmpfile(nfname,0600,"/tmp/dirnamXXXXXXXX")) < 0)
	    return PROR_BADTMP ;

	if ((rs = bopen(nfp,nfname,"rwct",0600)) < 0) {

	    rs = PROR_NAMEOPEN ;
	    goto bad1 ;
	}

	if ((rs = bopen(nnfp,nfname,"wct",0666)) < 0) {

	    rs = PROR_NAMEOPEN ;
	    goto bad2 ;
	}

/* set line buffering on the file so that we do not stall reading it */

	if ((rs = bcontrol(nnfp,BC_LINEBUF,0)) < 0) {

	    rs = PROR_NAMEOPEN ;
	    goto bad3 ;
	}

/* remove the temporary file now (for safer cleanup) */

	u_unlink(nfname) ;

/* preload the base directory name into the working directory name buffer */

	blen = strlen(bdp) ;

	if (blen > 0) {

	    strcpy(dirbuf,bdp) ;

	    strcat(dirbuf,"/") ;

	    blen += 1 ;

	}

	lbuf = dirbuf + blen ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) 
		debugprintf("process: starting dirbuf=%s dirlen=%d\n",
	    dirbuf,blen) ;
#endif

/* go through the loops */

	rs = 0 ;
	len = 0 ;
	while (f_seed || ((len = breadline(nfp,lbuf,llen)) > 0)) {

	    if (len && (lbuf[len - 1] == '\n')) 
		len -= 1 ;

	    lbuf[len] = '\0' ;
	    f_seed = FALSE ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("process: top of outer while\n%t\n",
	            lbuf,len) ;
#endif

#if	CF_DEBUG
	    if (pip->debuglevel > 1) debugprintf(
	        "process: outer loop dirbuf=%s len=%d\n",
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
		debugprintf("process: directory \"%s\"\n",
	        dirbuf) ;
#endif

/* is this a CAP special directory ? */

	    capbaselen = sfbasename(dirbuf,blen + len,&capbasep) ;

#if	CF_DEBUG
	    if (pip->debuglevel > 1) 
		debugprintf("process: CAP basename \"%t\"\n",
	        capbasep,capbaselen) ;
#endif

/* is this a CAP special directory ? */

	    f_cap = FALSE ;
	    if ((strncmp(capbasep,".finderinfo",11) == 0) ||
	        (strncmp(capbasep,".resource",9) == 0)) {

	        f_cap = TRUE ;
	        realdirlen = sfdirname(dirbuf,blen + len,&cp) ;

#if	CF_DEBUG
		strcpy(debugbuf,dirbuf) ;

		debugprintf("process: cp=\"%t\"\n",cp,realdirlen) ;

		debugprintf("process: dirname=\"%s\"\n",strdirname(debugbuf)) ;
#endif

	        strwcpy(realdirbuf,cp,realdirlen) ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1) {

		debugprintf("process: got a CAP special directory \"%s\"\n",
	            dirbuf) ;

		debugprintf("process: realdirbuf=\"%s\"\n",
	            realdirbuf) ;

		}
#endif /* CF_DEBUG */

	    } /* end if (CAP special directory) */

/* rock and roll ! */

	    while ((dlen = fsdir_read(&dir,&ds)) > 0) {

	        dep = &ds.entry ;
	        dnp = dep->d_name ;

/* do not play with the "special" entries ! (this is a fast way of deciding) */

	        if (*dnp == '.') {

	            if (dnp[1] == '\0') 
			continue ;

	            if ((dnp[1] == '.') && (dnp[2] == '\0'))
	                continue ;

	        }

#if	CF_DEBUG
		if (pip->debuglevel > 2)
	        debugprintf("process: continuing (inner) w/ name=%s\n",
	            dnp) ;
#endif

/* PROGRAM NOTE :
		
	At this point :
	= blen		base length (inluding a trailing slash character)
	= len		length of current directory from base
	= slen		will account for the slash character if needed

*/

/* is this a CAP special directory ? */

	        if (f_cap) {

/* yes, perform the CAP special processing */

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("process: CAP processing\n") ;
#endif

/* form the name of the data-fork file (write the zero byte in there) */

	            slen = 0 ;
	            if (realdirlen > 0) {

	                slen = 1 ;
	                realdirbuf[realdirlen] = '/' ;

	            }

	            strcpy(realdirbuf + realdirlen + slen,dnp) ;

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("process: checking for file=%s\n",
	                        realdirbuf) ;
#endif

/* do we do a remove operation or not ? */

	            if (((rs = u_stat(realdirbuf,&se)) < 0) &&
	                (rs == SR_NOENT)) {

/* make the name of the current file */

	                slen = 0 ;
	                if (len > 0) {

	                    slen = 1 ;
	                    lbuf[len] = '/' ;

	                }

	                strcpy(lbuf + len + slen,dnp) ;

/* remove the current entry (if we can) */

	                if (pip->f.verbose)
	                    bprintf(pip->ofp,"file fragment \"%s\"\n",
	                        dirbuf) ;

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("process: removing file=%s\n",
	                        dirbuf) ;
#endif

#if	CF_REMOVE
		if (((! pip->f.verbose) || pip->f.remove) && 
			(pip->debuglevel == 0))
	                unlink(dirbuf) ;
#endif
		
	                if (pip->f.verbose)
	                    bprintf(pip->ofp,"removing file \"%s\"\n",
	                        dirbuf) ;

	                ndel += 1 ;
	            }

	        } else {

/* prepare to 'stat' this entry */

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("process: regular processing\n") ;
#endif

	            slen = 0 ;
	            if (len > 0) {

	                slen = 1 ;
	                strcpy(lbuf + len,"/") ;

	            }

	            strcpy(lbuf + len + slen,dnp) ;

/* if we can't 'stat' it, then it cannot be too important to us ! */

	            if (u_stat(dirbuf,&se) < 0) 
			continue ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("process: 3 con\n") ;
#endif

/* from now on, we only want to deal with directories that we find */

	            if (! S_ISDIR(se.st_mode))
			continue ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("process: about to process a directory\n") ;
#endif

/* see if the real user can access the directory */

	            if (u_access(dirbuf,R_OK) < 0) 
			continue ;

/* OK, we are free, prepare to write out information on this directory */

	            ndir += 1 ;
	            dlen = strlen(dnp) ;

/* write out the directory name as a coded line */

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("process: writing a name\n%s\n",
	                    lbuf) ;
#endif

	            wlen = len + slen + dlen + 1 ;
	            rs = bprintf(nnfp,"%t\n",lbuf,(wlen - 1)) ;

		if (rs < wlen) {

#if	CF_DEBUG
	                if (pip->debuglevel > 1)
	                    debugprintf("process: bad write (rs %d)\n",
	                        rs) ;
#endif

	                break ;
	            }

	        } /* end if (CAP special or not) */

	    } /* end while (inner) */

	    fsdir_close(&dir) ;

	    if (rs < 0)
		break ;

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
/* end subroutine (process) */




/* mbrewrite */

/* subroutine to delete messages from a mailbox and write it back */


#define	CF_DEBUG	0		/* switchable debug print-outs */
#define	CF_MAILBOXDEL	0		/* mailbox delete */


/* revision history:

	= 94/01/20, David A­D­ Morano
        Oh, what a mess. I took over this code from nobody since it has been
        lying dormant for about 13 or more years! I have fixed countless
        problems and made a couple of enhancements.

	= 01/01/03, David A­D­ Morano
        I intend to provide some redimentary locking mechanism on the mailbox
        file. This would have been handled automatically if plans proceeded to
        move to the new mailbox object that was planned back in 1994! It is
        amazing how the years fly by with no progress on some old outstanding
        issues! Needless to say, I am rather forced to do this hack since I
        really do need correct mailbox file locking from time to time.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	removes messages marked for deletion in current mailbox
   then closes the file.   returns 0 if successful, 1 if error.


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<stdio.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#undef	BUFLEN
#define	BUFLEN		(8 * 1024)
#define	TO_LOCK		15		/* seconds */


/* external subroutines */

extern int	mkpath1(char *,const char *) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	lockfile(int,int,offset_t,offset_t,int) ;

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */

extern struct mailbox	mb ;


/* local (forward) subroutines */


/* local variables */


/* exported subroutines */


int mbrewrite(pip,mbp)
struct proginfo	*pip ;
struct mailbox	*mbp ;
{
	sigset_t	oldsigmask, newsigmask ;

	struct ustat	sb ;

	offset_t	offset ;

	long	mlen ;

	int	rs, i, n, permit ;
	int	flen ;
	int	len, rlen ;
	int	ifd, ofd ;
	int	newmessnum ;
	int	msgsize ;
	int	sl ;
	int	f_havestuff = FALSE ;

	char	template[MAXPATHLEN + 1] ;
	char	tempfile[MAXPATHLEN + 1] ;
	char	dot_mailbox[MAXPATHLEN + 1], *ifname ;
	char	buf[BUFLEN + 1], *bp ;
	char	*cp, *cp2 ;


#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	    debugprintf("mbrewrite: ent\n") ;
#endif

	if (mbp->mfp == NULL)
		return SR_FAULT ;

	if (mbp->magic != MB_MAGIC)
	    return SR_NOTOPEN ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
		debugprintf("mbrewrite: we have an open mailbox=%s\n",
			mbp->mailfile) ;
#endif

/* close the old mailbox file (possibly open since initial mailbox setup) */

	fclose(curr.fp) ;

	rs = bclose(mbp->mfp) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
		debugprintf("mbrewrite: bclose rs=%d\n",rs) ;
#endif

/* open the mail file */

	rs = open(mbp->mailfile,O_RDWR,0666) ;

	ifd = rs ;
	if (rs < 0)
		goto bad0 ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	    debugprintf("mbrewrite: opened current mail file \"%s\"\n",
	        mbp->mailfile) ;
#endif

/* try to lock the mailbox file */

	rs = lockfile(ifd,F_WLOCK,0L,0L,TO_LOCK) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
	    debugprintf("mbrewrite: lockfile() rs=%d\n",rs) ;
#endif

	if (rs < 0) {

#if	CF_DEBUG
	if (pip->debuglevel >= 2)
	    debugprintf("mbrewrite: lockfile() rs=%d\n",rs) ;
#endif

		return rs ;
	}

/* OK, we're good for a GO! */

	u_fstat(ifd,&sb) ;

	mkpath1(template,"/tmp/mbrewriteXXXXXX") ;

	tempfile[0] = '\0' ;
	rs = mktmpfile(template,sb.st_mode,tempfile) ;

	if (rs < 0)
		goto bad1 ;

#ifdef	COMMENT
	u_chmod(tempfile,0660) ;
#endif

/* create tempfile containing unmarked messages */

	ofd = u_open(tempfile,O_RDWR,0666) ;

	if (ofd < 0) {

#if	CF_DEBUG
	if (pip->debuglevel >= 2)
	        debugprintf("mbrewrite: bad 'open(2)' on temporary file (%d)\n",
	            ofd) ;
#endif

	    u_close(ifd) ;

	    rs = BAD ;
	    goto err1 ;
	}

	(void) u_fchown(ofd,-1,sb.st_gid) ;

/* do it */

	newmessnum = 0 ;
	offset = 0 ;
	for (i = 0 ; i < mbp->total ; i += 1) {

	    if (messdel[i] == 0) {

	        u_seek(ifd,(offset_t) messbeg[i],SEEK_SET) ;

	        mlen = 0 ;
	        while (mlen < mlength[i]) {

	            rlen = MIN(BUFLEN,mlength[i] - mlen) ;
	            if ((rs = u_read(ifd,buf,rlen)) <= 0) 
			break ;

		    len = rs ;
	            if ((rs = u_write(ofd,buf,len)) < len)
	                break ;

	            mlen += len ;

	        } /* end while */

	        if (mlen != mlength[i]) {

#if	CF_DEBUG
	if (pip->debuglevel >= 2) {

	                debugprintf(
	                    "mbrewrite: could not rewrite mailbox (1)\n") ;

	                debugprintf("	mlen=%d mlength[%d]=%d\n",
				mlen,i,mlength[i]) ;

	                debugprintf("	rlen=%d len=%d rs=%d\n",
				rlen,len,rs) ;

	            }
#endif /* CF_DEBUG */

	            if (pip->f.log) 
			logfile_printf( &pip->lh,
			    "could not mbrewrite mailbox, length mismatch\n",
	                	rs) ;

	            u_close(ifd) ;

	            u_close(ofd) ;

	            rs = BAD ;
	            goto err1 ;
	        }

	        messbeg[i] = offset ;
	        messend[i] = offset + mlength[i] - 1 ;
	        f_havestuff = TRUE ;
	        newmessnum += 1 ;
	        offset += mlen ;

	    } /* end if */

	} /* end for */


/* always copy the remainder of the file to the new mailbox file */

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
		debugprintf("mbrewrite: copy remainder of file?\n") ;
#endif

	if (mbp->total > 0)
	    rs = u_fstat(ifd,&sb) ;


	if ((mbp->total > 0) && (rs >= 0) && (sb.st_size > mbp->mblen)) {

	    u_seek(ifd,mbp->mblen,SEEK_SET) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
		debugprintf("mbrewrite: copying remainder of file!\n") ;
#endif

	    rs = FALSE ;
	    len = 0 ;
	    while ((rs = u_read(ifd,buf,BUFLEN)) > 0) {

		sl = rs ;
	        f_havestuff = TRUE ;
	        if ((rs = u_write(ofd,buf,sl)) < sl)
	            break ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
		debugprintf("mbrewrite: copied stuff at end %d\n",len) ;
#endif

	        len += sl ;

	    } /* end while */

	    if (rs < 0) {

#if	CF_DEBUG
	if (pip->debuglevel >= 2) {

	            debugprintf(
	                "mbrewrite: could not mbrewrite mailbox file (2)\n") ;

	            debugprintf("	rlen=%d len=%d rs=%d\n",rlen,len,rs) ;

	        }
#endif

	        if (pip->f.log) 
		    logfile_printf(&pip->lh,
			"could not mbrewrite mailbox (rs %d)\n",
	            rs) ;

	        u_close(ifd) ;

	        u_close(ofd) ;

	        rs = BAD ;
	        goto err1 ;
	    }

	    if (len > 0)
	        logfile_printf(&pip->lh,
	            "tail copy, len=%d\n",len) ;

	} /* end if (remainder of file) */


/* OK, now we copy the temporary file back to the original mailbox file */

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
		debugprintf("mbrewrite: copying back to the original!\n") ;
#endif

	if (f_havestuff) {

	u_seek(ifd,0L,SEEK_SET) ;

	rs = u_seek(ofd,0L,SEEK_SET) ;

	if (rs >= 0)
	    rs = uc_copy(ofd,ifd,-1) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
		debugprintf("mbrewrite: uc_copy() rs=%d\n",rs) ;
#endif

/* if there is still some left in the original file, truncate it */

	flen = rs ;
	if (rs >= 0) {

		rs = uc_ftruncate(ifd,(offset_t) flen) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
		debugprintf("mbrewrite: uc_ftruncate() rs=%d\n",rs) ;
#endif

	}

	} /* end if (we had stuff in the final mailbox file) */


/* close and get out! */

	u_close(ifd) ;			/* and unlocks the file also! */

	u_close(ofd) ;


#ifdef	COMMENT

/* create the revised mailbox index file if there were any messages for it */

	cp = dot_mailbox ;
	n = sfdirname(mbp->mailfile,-1,&cp2) ;

	strncpy(cp,cp2,n) ;

	cp += n ;
	*cp++ = '/' ;
	*cp++ = '.' ;
	*cp = '\0' ;
	strcat(cp,strbasename(mbp->mailfile)) ;

	ifname = cp ;

#endif /* COMMENT */


/* there were messages: save permissions, delete old, make new */

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
		debugprintf("mbrewrite: about to decide if we have stuff\n") ;
#endif

	if (f_havestuff) {

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
		debugprintf("mbrewrite: messages - old_total=%d new_total=%d\n",
	        mbp->total,newmessnum) ;
#endif

	} else {	

/* we don't have any data left -- just delete */

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
		debugprintf("mbrewrite: deleting mailbox file\n") ;
#endif

#if	CF_MAILBOXDEL
	    u_unlink(mbp->mailfile) ;
#else
	uc_truncate(mbp->mailfile,0L) ;
#endif

	    u_unlink(ifname) ;

	} /* end if (had stuff left in mailbox or not) */


/* cleanup */

#if	CF_DEBUG
	if (pip->debuglevel >= 3)
		debugprintf("mbrewrite: exiting newmessages=%d\n",
		newmessnum) ;
#endif

	rs = newmessnum ;

/* come here on back stuff */
err1:
ret1:
	if (tempfile[0] != '\0')
		u_unlink(tempfile) ;

ret0:
	return rs ;

bad1:
	u_close(ifd) ;

bad0:
	return rs ;
}
/* end subroutine (mbrewrite) */




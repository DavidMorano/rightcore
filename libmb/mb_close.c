/* mb_close */


#define	DEBUG		1
#define	NEWMAIL		0


/****************************************************************************

	** Mailbox Object Manipulation **

	These routines operate on a milabox object.

	= open (mailbox)
		- read only
		- read/write
		- does not exist, et cetera
	
	= close (mailbox)
	= delete message
	= undelete message
	= read message)
	= write message ?

	We open a mailbox by first trying to capture a record lock on
	the mailbox file itself.  We try for a write lock (which will
	indicate that we can later possibly update the file) and then
	we switch to a read lock so that others can join in also.  If we
	can only get a read lock, or if the user only requested read
	priviledges, then we just attempt a read lock directly.

	Once we have a lock on the mailbox file itself, we move to
	check for the existence of various index/caching like files.
	The main additional file is the mailbox index file which
	contains an already computed index on the mailbox file itself.
	There is much word in verifying that the index file is a valid
	one.  It must have a modification time later than the mailbox
	file and have the valid version recorded at its beginning.


****************************************************************************/



#include	<sys/types.h>
#include	<sys/stat.h>
#include	<sys/param.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<signal.h>
#include	<string.h>
#include	<ctype.h>
#include	<errno.h>

#include	<bfile.h>
#include	<baops.h>

#include	"localmisc.h"
#include	"mb.h"



#define		FROM_COLON	0



/* external subroutines */

extern int	sprintf() ;

char		*strbasename() ;


/* external variables */

extern struct mailbox	mb ;


/* local (forward) subroutines */


/* local data */

static int		tempmessord[MAXMESS + 1] ;

static int		ptord ;		/* to make it compile */



/************************************************************************

 Set up the message pointers for the specified mailbox file.
   Function returns 0 if there was at least one .
		    1 if the mailbox could not be opened.
		    2 if no messages are in the maibox.
   mailbox file is opened here and remains open until rdebugwrite.


**************************************************************************/



#define DELIMITER	"From "		/* inter message separator */
#define DELLEN		5		/* length of delimiter */
#define	MB_MB_COPYLEN	2048



/* local subroutines */

static int	expunge() ;


/* local variables */

static struct ustat	errbuf ;



/* 
	removes messages marked for deletion in current mailbox
   then closes the file.   returns 0 if successful, 1 if error.
*/

int mb_close(mbp)
struct mailbox	*mbp ;
{
	FILE		*dotfp ;

	sigset_t	oldsigmask, newsigmask ;

	offset_t	offset ;

	long	mlen ;

	int	len, rlen ;
	int	mfd, tfd ;
	int	rs ;
	int	i, n, permit ;
	int	newmessnum ;
	int	msgsize ;
	int	f_writeback = FALSE ;

	char	tempfile[MAXPATHLEN] ;
	char	dot_mailbox[MAXPATHLEN], *ifname ;
	char	buf[MB_COPYLEN + 1], *cp ;


	tempfile[0] = '\0' ;
	if ((mbp->magic != MB_MAGIC) || (mbp->fp == NULL)) {

	    rs = RMB_BADOPEN ;
	    goto baddone ;
	}

#if	DEBUG
	    if (BATST(UOV_DEBUG,g.uo)) debugprintf(
	        "mb_close: continuing to delete the marked messages\n") ;
#endif

/* open the mail file */

	if ((mfd = open(mbp->mailfile,O_RDWR,0666)) < 0) {

#if	DEBUG
	    if (BATST(UOV_DEBUG,g.uo)) debugprintf(
	        "mb_close: bad 'open(2)' on current mail file (%d)\n",mfd) ;
#endif

	    rs = RMB_BADOPEN ;
	    goto baddone ;
	}

#if	DEBUG
	if (BATST(UOV_DEBUG,g.uo))
	    debugprintf("mb_close: opened current mail file \"%s\"\n",
	        mbp->mailfile) ;
#endif

/* the 'tempfile' can't be in '/tmp' and it can't be "link"ed to */

	strcpy(tempfile,g.folder) ;

	strcat(tempfile,"/rdebugwriteXXXXXX") ;

	mktemp(tempfile) ;

	chmod(tempfile,0600) ;

/* create tempfile containing unmarked messages */

	if ((tfd = open(tempfile,O_WRONLY | O_CREAT | O_TRUNC,0666)) < 0) {

#if	DEBUG
	    if (BATST(UOV_DEBUG,g.uo)) debugprintf(
		"mb_close: bad 'open(2)' on temporary mail file (%d)\n",
		tfd) ;
#endif

	    ds_info("can't update mailbox ; no deletions made\n") ;

	    close(mfd) ;

	    rs = -1 ;
	    goto baddone ;
	}

	newmessnum = 0 ;
	offset = 0 ;
	for (i = 0 ; i < mbp->total ; i += 1) {

	    if (messdel[i] == 0) {

	        lseek(mfd,messbeg[i],SEEK_SET) ;

	        mlen = 0 ;
	        while (mlen < mlength[i]) {

	            rlen = MIN(MB_COPYLEN,mlength[i] - mlen) ;
	            if ((len = read(mfd,buf,rlen)) <= 0) break ;

	            if ((rs = write(tfd,buf,len)) < len) break ;

	            mlen += len ;
	        }

	        if (mlen != mlength[i]) {

#if	DEBUG
	            if (BATST(UOV_DEBUG,g.uo)) {

	                debugprintf(
	                    "mb_close: could not write temporary mail file\n") ;

	                debugprintf("	rlen=%d len=%d rs=%d\n",rlen,len,rs) ;
	            }
#endif

	            ds_info("could not write temporary mail file\n") ;

	            close(mfd) ;

	            close(tfd) ;

	            rs = -1 ;
	            goto baddone ;
	        }

	        messbeg[i] = offset ;
	        messend[i] = offset + mlength[i] - 1 ;
	        f_writeback = TRUE ;
	        newmessnum += 1 ;
	        offset += mlen ;

	    } /* end if */

	} /* end for */

/* always copy the remainder of the file to the new mailbox file */

#if	NEWMAIL
	i = mbp->total - 1 ;
	lseek(mfd,messbeg[i] + mlength[i],SEEK_SET) ;

	rs = FALSE ;
	while ((len = read(mfd,buf,MB_COPYLEN)) > 0) {

#if	DEBUG
	    if (BATST(UOV_DEBUG,g.uo)) debugprintf(
	        "mb_close: got some new mail\n") ;

	    logfile_printf("got some new mail\n") ;
#endif

	    f_writeback = TRUE ;
	    if (rs = (write(tfd,buf,len) < len)) break ;

	}

	if (rs) {

#if	DEBUG
	    if (BATST(UOV_DEBUG,g.uo)) {

	        debugprintf(
	            "mb_close: could not write temporary mail file\n") ;

	        debugprintf("	rlen=%d len=%d rs=%d\n",rlen,len,rs) ;

	    }
#endif

	    ds_info("could not write temporary mail file\n") ;

	    close(mfd) ;

	    close(tfd) ;

	    rs = -1 ;
	    goto baddone ;
	}
#endif


/* create the revised mailbox index file if there were any messages for it */

	cp = dot_mailbox ;
	n = dirname(mbp->mailfile) ;

	strncpy(cp,mbp->mailfile,n) ;

	cp += n ;
	*cp++ = '/' ;
	*cp++ = '.' ;
	*cp = '\0' ;
	strcat(cp,strbasename(mbp->mailfile)) ;

	ifname = cp ;

/* were messages: save permissions, delete old, make new */

	if (f_writeback) {

#if	DEBUG
	    if (BATST(UOV_DEBUG,g.uo)) debugprintf(
	        "mb_close: doing the link stuff - old_total=%d new_total=%d\n",
	        mbp->total,newmessnum) ;
#endif

/* start of indivisible operation */

	    newsigmask = 0 ;
	    newsigmask |= sigmask(SIGHUP) ;

	    newsigmask |= sigmask(SIGTERM) ;

	    newsigmask |= sigmask(SIGINT) ;

	    newsigmask |= sigmask(SIGQUIT) ;

	    sigprocmask(SIG_BLOCK,&newsigmask,&oldsigmask) ;


/* continue with operation */

	    stat(mbp->mailfile,&errbuf) ;

	    permit = errbuf.st_mode & 0777 ;

	    rename(tempfile,mbp->mailfile) ;

	    chmod(mbp->mailfile,permit) ;

#ifdef	COMMENT
	    lseek(tfd,(offset_t) 0,SEEK_SET) ;

	    lseek(mfd,(offset_t) 0,SEEK_SET) ;

	    rs = FALSE ;
	    while ((len = read(tfd,buf,MB_COPYLEN)) > 0) {

	        if (rs = (write(mfd,buf,len) < len)) break ;

	    }

	    if (rs) {

#if	DEBUG
	    if (BATST(UOV_DEBUG,g.uo)) debugprintf(
	        "mb_close: error during over write (errno %d)\n",
	        errno) ;
#endif

	        stat(mbp->mailfile,&errbuf) ;

	        permit = errbuf.st_mode & 0777 ;
	        unlink(mbp->mailfile) ;

	        link(tempfile,mbp->mailfile) ;

	        chmod(mbp->mailfile,permit) ;

	    }
#endif


/* turn interrupts, etc back on */

	    sigprocmask(SIG_SETMASK,&oldsigmask,NULL) ;


/* write out the mailbox index file */

#ifdef	COMMENT
	    dotfp = fopen(ifname, "w") ;

	    putw(msgsize, dotfp) ;

	    putw(newmessnum, dotfp) ;

	    for (i = 0 ; i <= mbp->total ; i += 1) {

	        if (messdel[i] != 0) continue ;

	        putw(messbeg[i], dotfp) ;
	        putw(messend[i], dotfp) ;
	        putw(messlen[i], dotfp) ;
	    }

	    fclose(dotfp) ;
#endif

	} else {	/* were none: just delete */

	    unlink(mbp->mailfile) ;

	    unlink(ifname) ;

	}

	rs = newmessnum ;
	close(tfd) ;

/* come here when done */
done:
	close(mfd) ;

/* close the old mailbox (possibly open since initial mailbox setup) */
baddone:
	fclose(curr.fp) ;

/* cleanup */

	if (*tempfile) unlink(tempfile) ;

	return rs ;
}
/* end subroutine (mb_close) */



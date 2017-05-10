/* mb_open */


#define	CF_DEBUG	1
#define	CF_NEWMAIL	0


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
#include	<fcntl.h>
#include	<signal.h>
#include	<string.h>
#include	<curses.h>
#include	<ctype.h>
#include	<errno.h>

#include	<bfile.h>
#include	<baops.h>

#include	"localmisc.h"
#include	"mailbox.h"



/* local defines */

#define	FROM_COLON	0

#ifndef	BUFLEN
#define	BUFLEN		(MAXPATHLEN + 12)
#endif



/* external subroutines */

extern int	sfshrink(const char *,int,char **) ;

extern char	*strbasename(char *) ;


/* external variables */

extern struct global	g ;

extern struct mailbox	mb ;


/* local (forward) subroutines */


/* local data */



/************************************************************************

 Set up the message pointers for the specified mailbox file.
   Function returns 0 if there was at least one .
		    1 if the mailbox could not be opened.
		    2 if no messages are in the maibox.
   mailbox file is opened here and remains open until rdebugwrite.


**************************************************************************/



#define DELIMITER	"From "		/* inter message separator */
#define DELLEN		5		/* length of delimiter */




int mb_open(mbp,mf,oflag)
struct mailbox	*mbp ;
char		*mf ;
int		oflag ;
{
	struct ustat	stat_mb, stat_mbi ;

	struct flock	flock_mb ;

	bfile	ifile, *ifp = &ifile ;

	offset_t	maillen ;
	offset_t	offset ;	/* keeps track of location in file */

	int	rs, i ;
	int	iclen, clen, clines ;
	int	len, l ;
	int	nl, lines, baseline ;
	int	mfd ;			/* mailbox file descriptor */
	int	f_bol, f_eol ;
	int	f_iexists = FALSE ;		/* index exists */
	int	f_iopen = FALSE ;		/* index open */
	int	f_igood = FALSE ;		/* index is good ! */
	int	f_ireadonly = FALSE ;		/* index read-only */
	int	f_writelock = FALSE ;
	int	f_startmessage ;
	int	f_header = FALSE ;
	int	f_clen = FALSE ;
	int	f_clines ;
	int	f_text ;

	char	linebuf[LINELEN + 1] ;	/* temporary storage line read in */
	char	ifname[MAXPATHLEN + 1] ;
	char	*bn ;
	char	*cp, *cp2 ;


	mbp->total = 0 ;
	for (i = 0 ; i < MB_OVERLAST ; i += 1) mbp->stat[i] = 0 ;

	strcpy(mbp->mailfile,mf) ;

	bn = mbp->mailbox = strbasename(mbp->mailfile) ;

/* open the mailbox ; if it couldn't be opened, return BAD */

	mbp->fp = NULL ;
	mbp->magic = 0 ;
	mbp->mblen = 0 ;
	if (bopen(mbp->mfp,mf,"rw",0666) < 0) {

		BASET(MBV_READONLY,mbp->stat) ;
		if (bopen(mfp->mfp,"r",0666) < 0) 
			goto badopen ;

	}

	mbp->fp = curr.fp ;
	if (bcontrol(mbp->mfp,BC_GETFD,&mfd) < 0) 
		goto badfd ;

/* get some information on the mailbox file */

	if ((fstat(mfd, &stat_mb) < 0) || (! S_ISREG(stat_mb.st_mode)))
		goto badfd ;

/* OK, we finally consider this mailbox opened */

	mbp->magic = MB_MAGIC ;

/* if we are "read-only" go immediately to establish a read-only lock */
lock:
	if (BATST(MBV_READONLY,mbp->stat) goto readlock ;

/* try to write-lock the mailbox file */
writelock:
	flock_mb.l_type = F_WRLCK ;
	flock_mb.l_whence = SEEK_SET ;
	flock_mb.l_start = 0 ;
	flock_mb.l_len = stat_mb.st_size ;
	if (fcntl(mfd,F_SETLK,&flock_mb) < 0) {

		if ((errno == EAGAIN) || (errno == EACCESS)) 
			goto readlock ;

		goto badfd ;

	} /* end if */

	f_writelock = TRUE ;

/* got the write lock on the mailbox */

/* start the involved procedure for insuring that we do not read a bad index */
checkindex:

/* make the name of the index file for this mailbox */

	if ((i = dirname(mf)) > 0)
		bufprintf(ifname,MAXPATHLEN,
			"%t/.MI%s",mf,i,bn) ;

	else
		bufprintf(ifname,MAXPATHLEN,
			".MI%s",bn) ;

#if	CF_DEBUG
	if (BATST(UOV_CF_DEBUG,g.uo))
	    debugprintf("setup 10\n") ;
#endif

/* verify that the mail index file is up-to-date */

	stat_mb.st_size = 0 ;
	if (stat(ifname, &stat_mbi) == 0) {

		f_iexists = TRUE ;

#if	CF_DEBUG
	    if (BATST(UOV_CF_DEBUG,g.uo))
	        debugprintf("setup 10a\n") ;
#endif

	    if ((stat_mb.st_mtime <= stat_mbi.st_mtime) &&
	        S_ISREG(stat_mbi.st_mode)) {

#if	CF_DEBUG
	        if (BATST(UOV_CF_DEBUG,g.uo))
	            debugprintf("setup 10b\n") ;
#endif

/* try to open the index file */

	        if (bopen(ifp,ifname, "rw",0666) >= 0) f_iopen = TRUE ;

		if ((! f_iopen) && (bopen(ifp,ifname, "r",0666) >= 0)) {

			f_ireadonly = TRUE ;
	 		f_iopen = TRUE ;

		}

#if	CF_DEBUG
	            if (BATST(UOV_CF_DEBUG,g.uo))
	                debugprintf("setup 10c\n") ;
#endif

/* if the mailbox index is open, verify its magic number, version, et cetera */

		if (f_iopen) {

	            linebuf[0] = '\0' ;
	            len = breadline(ifp,linebuf,LINELEN) ;

	            if (strncmp(linebuf,MAGIC,strlen(MAGIC)) == 0) {

	                linebuf[0] = '\0' ;
	                len = breadline(ifp,linebuf,LINELEN) ;

	                if (strncmp(linebuf,VERSION,strlen(VERSION)) == 0) {

				len = breadline(ifp,linebuf,LINELEN) ;

	                if (strncmp(linebuf,bn,strlen(bn)) == 0) {

	                    len = bread(ifp,&maillen,sizeof(long)) ;

	                    if ((len == sizeof(long)) && 
				(maillen == stat_mb.st_size)) {

	                        f_igood = TRUE ;

#if	CF_DEBUG
	                        if (BATST(UOV_CF_DEBUG,g.uo))
	                            debugprintf("setup 10f\n") ;
#endif
	                    }
			}
	                }
	            }

	        } /* end if (of opening the index file) */
	    }

	} /* end if (for the good 'stat') */

#if	CF_DEBUG
	if (BATST(UOV_CF_DEBUG,g.uo))
	    debugprintf("setup 11\n") ;
#endif

/* if we do not have a good index file, rebuild it */

	if (f_igood) goto mapindex:

/* rebuild the index file */

	if (f_iopen && (! f_ireadonly)) bseek(ifp,0,SEEK_SET) ;

/* rebuild the mailbox index file */

	if (! BATST(MBV_READING,mbp->stat)) {

#ifdef	COMMENT
		if (f_iexists) unlink(ifname) ;
#endif

		if (bopen(ifp,ifname,"rwct",0666) >= 0) f_iopen = TRUE ;

	}

/* if not already open, make a temporary name to use as an index file */

	if (! f_iopen) {

		bufprintf(ifname,MAXPATHLEN,
			"/tmp/mbiXXXXXXXXXXX") ;

		if (bopen(ifp,ifname,"rw",0666) < 0) 
			goto couldnot ;

	}

#if	CF_DEBUG
	    if (BATST(UOV_CF_DEBUG,g.uo))
	        debugprintf("setup 12\n") ;
#endif

/* parse out this mailbox file the hard way */

	    i = 0 ;
	iclen = 0 ;
	    offset = 0 ;
	    baseline = 0 ;
	    lines = 0 ;
		f_bol = TRUE ;
	    while ((len = breadline(mbp->mfp,linebuf,LINELEN)) > 0) {

		f_eol = FALSE ;
		if (linebuf[len - 1] == '\n') f_eol = TRUE ;

	        linebuf[len] = '\0' ;
	        cp = linebuf + MIN(DELLEN,len) ;
	        while (ISWHITE(*cp)) cp += 1 ;

/* check for specific "UNIX" type mail message envelope */

		f_startmessage = FALSE ;
		if (f_header || (! f_clen) || (iclen > clen))
			f_startmessage = 
			((strncmp(linebuf, DELIMITER, DELLEN) == 0) &&
	            	isalpha(*cp)) ;

		if (f_startmessage) {

	            f_header = TRUE ;

#if	CF_DEBUG
	            if (BATST(UOV_CF_DEBUG,g.uo))
	                debugprintf("message %d base line is %ld\n",i,baseline) ;
#endif

/* record where message begins */

	            messbeg[i] = offset ;

/* previous mess ends 1 byte before new mess begins */

	            if (i > 0) {

	                messend[i - 1] = offset - 1 ;
	                messlen[i - 1] = (baseline <= 0) ? ABS(baseline) : 
	                    (lines - baseline) ;
	                mlength[i - 1] = offset - messbeg[i - 1] ;

#if	CF_DEBUG
	                if (BATST(UOV_CF_DEBUG,g.uo))
	                    debugprintf("message %d lines=%ld\n",
	                        i - 1,messlen[i - 1]) ;
#endif

	            }

	            i += 1 ;

/* set the boolean falg defaults for the new message */

	            f_text = TRUE ;
	            f_clen = FALSE ;
	            f_clines = FALSE ;

	        } else if (f_header) if (l = hmatch("content-length",linebuf)) {

	            l = sfshrink(linebuf + l,len - l,&cp) ;

	            if (cfdec(cp,l,&clen) >= 0)
	                f_clen = TRUE ;

	        } else if (l = hmatch("content-lines",linebuf)) {

#if	CF_DEBUG
	            if (BATST(UOV_CF_DEBUG,g.uo)) debugprintf(
	                "mailbox: got some content-lines \"%s\"\n",
	                linebuf + l) ;
#endif

	            l = sfshrink(linebuf + l,len - l,&cp) ;

	            if (cfdec(cp,l,&clines) >= 0)
	                f_clines = TRUE ;

	        } else if ((! f_clines) && (l = hmatch("lines",linebuf))) {

#if	CF_DEBUG
	            if (BATST(UOV_CF_DEBUG,g.uo)) debugprintf(
	                "mailbox: got some 'lines' \"%s\"\n",
	                linebuf + l) ;
#endif

	            l = sfshrink(linebuf + l,len - l,&cp) ;

	            if (cfdec(cp,l,&clines) >= 0)
	                f_clines = TRUE ;

	        } else if ((! f_clines) && (l = hmatch("x-lines",linebuf))) {

#if	CF_DEBUG
	            if (BATST(UOV_CF_DEBUG,g.uo)) debugprintf(
	                "mailbox: got some x-lines \"%s\"\n",
	                linebuf + l) ;
#endif

	            l = sfshrink(linebuf + l,len - l,&cp) ;

	            if (cfdec(cp,l,&clines) >= 0)
	                f_clines = TRUE ;

	        } else if (l = hmatch("content-type",linebuf)) {

	            l = sfshrink(linebuf + l,len - l,&cp) ;

	            cp[l] = '\0' ;
#if	CF_DEBUG
	            if (BATST(UOV_CF_DEBUG,g.uo)) debugprintf(
	                "mailbox: got some content \"%s\"\n",
	                cp) ;
#endif

	            if ((cp2 = strpbrk(cp,"/;\t ")) != NULL) *cp2 = '\0' ;

	            if ((LOWER(*cp) != 't') || (strcmp(cp + 1,"ext") != 0)) {

	                f_text = FALSE ;

#if	CF_DEBUG
	                if (BATST(UOV_CF_DEBUG,g.uo)) debugprintf(
	                    "mailbox: got some non-text\n",
	                    clen) ;
#endif

	            }

	        } else if (linebuf[0] == '\n') {

/* we reached the end of the header portion of this message */

			iclen = -1 ;
	            f_header = FALSE ;
	            baseline = lines + 1 ;
	            if (f_clen && f_clines) {

	                baseline = - clines ;
	                offset += clen ;
	                bseek(mbp->mfp,(offset_t) clen,SEEK_CUR) ;

#if	CF_DEBUG
	                if (BATST(UOV_CF_DEBUG,g.uo)) debugprintf(
	                    "mailbox: found lines and used it\n",
	                    clen) ;
#endif

	            } else if (f_clen && (! f_text)) {

	                baseline = 0 ;
	                offset += clen ;
	                bseek(mbp->mfp,(offset_t) clen,SEEK_CUR) ;

#if	CF_DEBUG
	                if (BATST(UOV_CF_DEBUG,g.uo)) debugprintf(
	                    "mailbox: skipped over the content - clen=%ld\n",
	                    clen) ;
#endif

	            } else if (f_clines) {

	                baseline = - clines ;
	                nl = 0 ;
	                clen = 0 ;
	                while ((nl < clines) &&
	                    ((l = fgetline(curr.fp,linebuf,LINELEN)) > 0)) {

	                    if (linebuf[l - 1] == '\n') nl += 1 ;

	                    clen += l ;
	                }

	                offset += clen ;

#if	CF_DEBUG
	                if (BATST(UOV_CF_DEBUG,g.uo)) debugprintf(
	                    "mailbox: found lines (cheating) and used it %d\n",
	                    clines) ;
#endif
	            }

	        } /* end if (header parsing) */

/* find where ptr in file is positioned */

	        lines += 1 ;
	        offset += len ;
		iclen += len ;

	    } /* end while */

	    if (i > 0) {

	        messend[i - 1] = offset - 1 ;
	        messlen[i - 1] = 
	            (baseline <= 0) ? ABS(baseline) : (lines - baseline) ;
	        mlength[i - 1] = offset - messbeg[i - 1] ;

#if	CF_DEBUG
	        if (BATST(UOV_CF_DEBUG,g.uo))
	            debugprintf("message %d lines=%ld\n",
	                i - 1,messlen[i - 1]) ;
#endif

	    }

	    mbp->total = i ;	/* record the number of messages */
	    mbp->mblen = offset ;

	for (i = 0 ; i < mbp->total ; i += 1)
	    messdel[i] = 0 ;		/* no messages marked for deletion */

/* we now have an index file, we map it in */
mapindex:




#if	CF_DEBUG
	    if (BATST(UOV_CF_DEBUG,g.uo))
	        debugprintf("setup 20\n") ;
#endif

	    bread(ifp,&mbp->total,sizeof(long)) ;

	    for (i = 0 ; i < mbp->total ; i += 1) {

	        bread(ifp,&messbeg[i],sizeof(long)) ;

	        bread(ifp,&messend[i],sizeof(long)) ;

	        bread(ifp,&messlen[i],sizeof(long)) ;

	    }

	    bclose(ifp) ;


#if	CF_DEBUG
	if (BATST(UOV_CF_DEBUG,g.uo))
	    debugprintf("setup 30 w/ total=%d\n",mbp->total) ;
#endif

	if (mbp->total > MAXMESS) return RMB_TOOMANY ;

/* if there are no messages then return a 2 */

	if (mbp->total <= 0) return RMB_EMPTY ;

	return OK ;		/* successful completion */

/* control does not fall from above */
badopen:
	bufprintf(ifname,MAXPATHLEN,
		"%t/.MI%s",mf,i,bn) ;

	unlink(ifname) ;

	return RMB_VADOPEN ;

badfd:
	fclose(curr.fp) ;

	bclose(mfp->mfp) ;

	goto badopen ;

/* try to establish a read-lock on the mailbox file */
readlock:
	flock_mb.l_type = F_RDLCK ;
	flock_mb.l_whence = SEEK_SET ;
	flock_mb.l_start = 0 ;
	flock_mb.l_len = stat_mb.st_size ;
	if (fcntl(mfd,F_SETLK,&flock_mb) >= 0) {

		if ((errno == EAGAIN) || (errno == EACCESS)) {

			sleep(1) ;

			goto lock ;

		}

		goto badfd ;

	} /* end if */

	BASET(MBV_READING,mbp->stat) ;
	goto checkindex ;
}
/* end subroutine (mb_open) */




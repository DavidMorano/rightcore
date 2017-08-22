/* decode */

/* decode the message(s) */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* debug print-outs */
#define	CF_EXTRAMSG	1		/* handle extra message */
#define	CF_UNIFIED	1		/* perform unified check-summing */


/* revision history:

	= 1999-06-13, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1999 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine decodes the input data that was previously scrambled by
        this same program (in scramble mode).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<randomvar.h>
#include	<cksum.h>
#include	<localmisc.h>

#include	"ecmsg.h"
#include	"ecinfo.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#define	O_FLAGS		(O_WRONLY | O_CREAT | O_TRUNC)

#undef	BLOCKLEN
#define	BLOCKLEN	(NOPWORDS * sizeof(ULONG))

#ifndef	BUFLEN
#define	BUFLEN		BLOCKLEN
#endif


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	mktmpfile(char *,mode_t,const char *) ;

extern void	munge(PROGINFO *,int,ULONG *,ULONG *,ULONG *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */

struct msgdesc {
	CKSUM		sumer ;
	uint		cksum ;
	uint		msglen ;
	uint		tlen ;
	uint		i ;
	uint		wlen ;
} ;

struct msgstate {
	struct msgdesc	main ;
	struct msgdesc	extra ;
} ;


/* forward references */

static int	decode_start(PROGINFO *,struct msgstate *,int,int) ;
static int	decode_proc(PROGINFO *,struct msgstate *,char *,int) ;
static int	decode_finish(PROGINFO *,struct msgstate *) ;


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int decode(pip,rvp,fip,emp,ifd,ofd)
PROGINFO	*pip ;
RANDOMVAR	*rvp ;
FILEINFO	*fip ;
ECMSG		*emp ;
int		ifd, ofd ;
{
	struct msgstate	ms ;
	struct ustat	sb ;
	ULONG		key[NOPWORDS] ;
	ULONG		vector[NOPWORDS] ;
	ULONG		mask[NOPWORDS] ;
	ULONG		first[NOPWORDS] ;
	ULONG		scrambled[NOPWORDS] ;
	ULONG		clear[NOPWORDS] ;

#if	(! CF_UNIFIED)
	CKSUM		filesum, msgsum ;
#endif

	offset_t	offset ;
	time_t		filetime ;
	time_t		msgtime ;
	uint		val ;
	uint		filecksum, msgcksum ;
	int		rs = SR_OK ;
	int		rs1 ;
	int		i, j, rlen, len ;
	int		cl ;
	int		n = NOPWORDS ;
	int		sfd, mfd = -1 ;
	int		totalblocks ;
	int		datablocks ;
	int		filelen, msglen ;
	int		oflags ;
	int		f_spoolinput = FALSE ;
	char		*cp ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("decode: ent\n") ;
#endif

	if (pip == NULL) return SR_FAULT ;

	fip->mtime = 0 ;
	fip->len = 0 ;
	fip->cksum = 0 ;

	rs = u_fstat(ifd,&sb) ;
	if (rs < 0)
	    goto ret0 ;

/* do we have to spool the input file? */

	if ((! S_ISREG(sb.st_mode)) || (u_seek(ifd,0L,SEEK_SET) < 0)) {
	    int		tfd ;
	    char	infname[MAXPATHLEN + 2] ;
	    char	tmpfname[MAXPATHLEN + 2] ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("decode: input needs spooling\n") ;
#endif

	    mkpath2(infname,pip->tmpdname,"ecXXXXXXXXXXXX") ;

	    rs = mktmpfile(tmpfname,0600,infname) ;

	    if (rs < 0)
	        goto badtmpmk1 ;

	    rs = u_open(tmpfname,O_RDWR,0600) ;

	    u_unlink(tmpfname) ;

	    if (rs < 0)
	        goto badtmpopen1 ;

	    tfd = rs ;
	    rs = uc_copy(ifd,tfd,-1) ;

	    if (rs < 0) {
	        u_close(tfd) ;
	        goto badtmpcopy1 ;
	    }

	    ifd = tfd ;
	    f_spoolinput = TRUE ;
	    u_fstat(ifd,&sb) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("decode: spooled input file, rs=%d\n",rs) ;
#endif

	} /* end if (input was not a regular file) */

/* do we have a message file? */

	if ((pip->msgfname != NULL) && (pip->msgfname[0] != '\0')) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("decode: message file=%s\n",pip->msgfname) ;
#endif

	    rs = uc_open(pip->msgfname,O_FLAGS,0666) ;

	    if (rs < 0)
	        goto retearly ;

	    mfd = rs ;

	} /* end if (message file) */

/* continue with the decoding operation */

	totalblocks = sb.st_size / BLOCKLEN ;
	datablocks = totalblocks - 3 ;
	if (datablocks <= 0)
	    goto done ;

	offset = (totalblocks - 1) * BLOCKLEN  ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	    debugprintf("decode: datablocks=%d\n",datablocks) ;
	    debugprintf("decode: key offset=%ld\n",offset) ;
	}
#endif

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("decode: reading key at offset=%ld\n",offset) ;
#endif

/* do we have to stage the output file? */

	rs = u_fstat(ofd,&sb) ;
	if (rs < 0)
	    goto badoutstat ;

	rs = u_fcntl(ofd,F_GETFL,0) ;
	oflags = rs ;
	if (rs < 0)
	    goto badoutstat ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("decode: outfile oflags=%08x\n",oflags) ;
#endif

	if ((! S_ISREG(sb.st_mode)) || 
	    (u_seek(ofd,(offset_t) 0,SEEK_SET) < 0) ||
	    (! (oflags & O_RDWR))) {

	    char	infname[MAXPATHLEN + 2] ;
	    char	tmpfname[MAXPATHLEN + 2] ;


#if	CF_DEBUG
	    if (DEBUGLEVEL(2)) {
	        debugprintf("decode: output needs staging\n") ;
	        debugprintf("decode: O_RDWR=%u\n",
			((oflags & O_RDWR) ? 1 : 0)) ;
	    }
#endif

	    mkpath2(infname,pip->tmpdname,"ecXXXXXXXXXXXX") ;

	    rs = mktmpfile(tmpfname,0600,infname) ;
	    if (rs < 0)
	        goto badtmpmk2 ;

	    rs = u_open(tmpfname,O_RDWR,0600) ;

	    u_unlink(tmpfname) ;
	    if (rs < 0)
	        goto badtmpopen2 ;

	    sfd = rs ;

	} else
	    sfd = ofd ;

/* read in the last block, the key block */

	u_seek(ifd,offset,SEEK_SET) ;

	rs = u_read(ifd,key,BLOCKLEN) ;
	if (rs < 0)
	    goto badread ;

	u_seek(ifd,0L,SEEK_SET) ;

	for (i = 0 ; i < n ; i += 1) {
	    vector[i] = key[i] ;
	}

/* read the leader block */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("decode: reading leader block\n") ;
#endif

	rs = u_read(ifd,scrambled,BLOCKLEN) ;
	if (rs < 0)
	    goto badread ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("decode: munging leader block\n") ;
#endif /* CF_DEBUG */

	munge(pip,n,vector,scrambled,clear) ;

/* handle the first data block specially */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("decode: processing data blocks?\n") ;
#endif

	if (datablocks > 0) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("decode: we have data blocks\n") ;
#endif

	    rs = u_read(ifd,scrambled,BLOCKLEN) ;
	    len = rs ;
	    if (rs < 0)
	        goto badread ;

	    munge(pip,n,vector,scrambled,clear) ;

/* save the first block so that we can get the clear-text later */

	    for (i = 0 ; i < NOPWORDS ; i += 1) {
	        first[i] = clear[i] ;
	    }

/* don't bother writing anything for the first block yet, just skip */

	    u_seek(sfd,(offset_t) BLOCKLEN,SEEK_SET) ;

/* process the rest of the input file */

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("decode: processing rest of file\n") ;
#endif

	    rs = SR_OK ;
	    for (i = 1 ; i < datablocks ; i += 1) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("decode: about to read data block=%d\n",i) ;
#endif

	        rs = u_read(ifd,scrambled,BLOCKLEN) ;
	        len = rs ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("decode: i=%d u_read() rs=%d\n",
	                i,rs) ;
#endif

	        if (rs < BLOCKLEN) break ;

	        munge(pip,n,vector,scrambled,clear) ;

	        rs = u_write(sfd,clear,BLOCKLEN) ;
		if (rs < 0) break ;

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("decode: bottom for loop\n") ;
#endif

	    } /* end for (there was more input data to read) */

	    if (rs < 0) goto badread ;
	} /* end if (there were data blocks) */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("decode: done w/ data blocks\n") ;
#endif

/* read the mask block */

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("decode: reading mask block\n") ;
#endif

	rlen = BLOCKLEN ;
	rs = u_read(ifd,scrambled,rlen) ;
	len = rs ;
	if (rs < 0)
	    goto badread ;

	if (len < rlen) {
	    rs = SR_INVALID ;
	    goto badread ;
	}

	munge(pip,n,vector,scrambled,mask) ;

/* OK, decode the beginning of the first block with the mask block */

	for (i = 0 ; i < (n - pip->necinfo) ; i += 1) {
	    first[i] = first[i] ^ mask[i] ;
	}

/* decode the last few words of the mask block itself, with itself */

	j = 0 ;
	while (i < n) {
	    mask[i] = (mask[i] ^ mask[j++]) ;
	    i += 1 ;
	} /* end while */

/* get the file and extra message information */

	{
		struct ecinfo_data	m0 ;

		cp = (char *) (mask + (n - pip->necinfo)) ;
		cl = pip->necinfo * sizeof(ULONG) ;
		ecinfo_data(cp,cl,1,&m0) ;

		filelen = m0.filelen ;
		filetime = m0.filetime ;
		filecksum = m0.filesum ;

		msglen = m0.msglen ;
		msgtime = m0.msgtime ;
		msgcksum = m0.msgsum ;

	} /* end block */

	fip->mtime = filetime ;
	fip->len = filelen ;
	fip->cksum = filecksum ;

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("decode: mask file len=%u\n",filelen) ;
	    debugprintf("decode: mask file cksum=\\x%08x (%u)\n",
	        filecksum,filecksum) ;
	    debugprintf("decode: mask msglen=%u\n",msglen) ;
	}
#endif /* CF_DEBUG */

/* is the file length that we were given plausible? */

	if ((filelen < 0) || (filelen > (datablocks * BLOCKLEN))) {

	    rs = SR_INVALID ;
	    logfile_printf(&pip->lh,"implausible file length=\\x%08x\n",
	        filelen) ;

	    goto badfile ;
	}

/* is the file time that we were given plausible? */

	{
	    time_t	daytime = time(NULL) ;

	    if ((filetime < 0) ||
	        (filetime > (daytime + (3*24*3600)))) {

	        logfile_printf(&pip->lh,"implausible file time=\\x%08x\n",
	            filetime) ;

	        goto badfile ;
	    }

	} /* end block */

/* OK, now separate the file from the extra message */

#if	CF_DEBUG
	if (DEBUGLEVEL(4)) {
	    debugprintf("decode: separating data rs=%d\n",rs) ;
	    debugprintf("decode: sfd=%d odf=%d\n",sfd,ofd) ;
	}
#endif

#if	CF_UNIFIED

	rs = decode_start(pip,&ms,filelen,msglen) ;

#else /* CF_UNIFIED */

	rs = cksum_start(&filesum) ;

	cksum_start(&msgsum) ;

#endif /* CF_UNIFIED */

	if (rs < 0)
	    goto badfile ;

	{
	    int		fd ;
	    char	buf[BUFLEN + 1], *bp ;

#if	CF_UNIFIED

	    u_seek(ofd,(offset_t) 0,SEEK_SET) ;

	    decode_proc(pip,&ms,(char *) first,BLOCKLEN) ;

	    len = (sfd != ofd) ? ms.main.wlen : BLOCKLEN ;
	    rs = u_write(ofd,first,len) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(5))
	        debugprintf("decode: mfd=%d extra.wlen=%d\n", 
	            mfd,ms.extra.wlen) ;
#endif

	    if ((mfd >= 0) && (ms.extra.wlen > 0)) {

	        bp = (char *) first ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("decode: %d extra=>%t<\n", 
	                ms.extra.wlen,bp,strnlen(bp,MIN(15,ms.extra.wlen))) ;
#endif

	        rs = u_write(mfd,(bp + ms.extra.i),ms.extra.wlen) ;

	    }

#else /* CF_UNIFIED */

	    tlen = 0 ;
	    len = MIN(filelen,BLOCKLEN) ;
	    cksum_accum(&filesum,first,len) ;

	    tlen += rlen ;
	    u_seek(ofd,(offset_t) 0,SEEK_SET) ;

	    if (rs >= 0)
	        rs = u_write(ofd,first,len) ;

#endif /* CF_UNIFIED */

#if	CF_UNIFIED

	    if (sfd != ofd) {
	        u_seek(sfd,(offset_t) BLOCKLEN,SEEK_SET) ;
	    }

	    while ((ms.main.tlen < ms.main.msglen) ||
	        (ms.extra.tlen < ms.extra.msglen)) {

	        fd = (sfd != ofd) ? sfd : ofd ;
	        rs = u_read(fd,buf,BUFLEN) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("decode: u_read() rs=%d\n", rs) ;
#endif

	        len = rs ;
	        if (rs <= 0)
	            break ;

	        decode_proc(pip,&ms,buf,len) ;

	        if ((sfd != ofd) && (ms.main.wlen > 0)) {
	            rs = u_write(ofd,(buf + ms.main.i),ms.main.wlen) ;
		}

	        if ((rs >= 0) && (mfd >= 0) && (ms.extra.wlen > 0)) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("decode: %d extra=>%t<\n", 
	                    ms.extra.wlen,buf,
				strnlen(buf,MIN(15,ms.extra.wlen))) ;
#endif

	            rs = u_write(mfd,(buf + ms.extra.i),ms.extra.wlen) ;

	        }

	    } /* end while */

#if	CF_DEBUG
	    if (DEBUGLEVEL(5)) {
	        debugprintf("decode: main.tlen=%d\n",ms.main.tlen) ;
	        debugprintf("decode: extra.tlen=%d\n",ms.extra.tlen) ;
	    }
#endif

	    if (sfd != ofd) {

/* wipe out the stage file */

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("decode: wiping output stage file rs=%d\n",
	                rs) ;
#endif

	        u_seek(sfd,(offset_t) 0,SEEK_SET) ;

	        for (i = 0 ; i < datablocks ; i += 1) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("decode: wiping=%u n=%u\n",i,n) ;
#endif

	            for (j = 0 ; j < n ; j += 1) {
			scrambled[j] = 0 ;
	                randomvar_getulong(rvp,(scrambled + j)) ;
		    }

	            rs1 = u_write(sfd,scrambled,BLOCKLEN) ;

	            if (rs1 < 0) break ;
	        } /* end for */

	        u_close(sfd) ;
	    } /* end if (needed a "stage" file) */

	    if (sfd == ofd) {
	        uc_ftruncate(ofd,(offset_t) filelen) ;
	    }

#else /* CF_UNIFIED */

	    if (sfd == ofd) {

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("decode: output was readable\n") ;
#endif

	        rs = SR_OK ;
	        while (tlen < filelen) {

	            rlen = MIN((filelen - tlen),BUFLEN) ;
	            rs = u_read(ofd,buf,rlen) ;
	            len = rs ;
	            if (rs <= 0) break ;

	            cksum_accum(&filesum,buf,len) ;
	            tlen += len ;

	        } /* end while */

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("decode: out while rs=%d\n",rs) ;
#endif

	        if (rs < 0)
	            goto badoutread ;

/* now read the extra message if any */

#if	CF_EXTRAMSG

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("decode: msglen=%u\n",msglen) ;
#endif

	        tlen = 0 ;
	        while (tlen < msglen) {

	            rlen = MIN((msglen - tlen),BUFLEN) ;
	            rs = u_read(ofd,buf,rlen) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("decode: emsg u_read() rs=%d\n",rs) ;
#endif

	            len = rs ;
	            if (rs <= 0) break ;

	            cksum_accum(&msgsum,buf,len) ;

	            if (mfd >= 0) {
	                rs = u_write(mfd,buf,len) ;
		        if (rs < 0) break ;
		    }

	            tlen += len ;
	        } /* end while */

	        if (rs < 0)
	            goto badoutread ;
#endif /* CF_EXTRAMSG */

/* truncate the output file at the proper place */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("decode: truncating output file\n") ;
#endif

	        uc_ftruncate(ofd,(offset_t) filelen) ;

	    } else {

#if	CF_DEBUG
	        if (DEBUGLEVEL(5)) {
	            debugprintf("decode: output was NOT readable\n") ;
	            debugprintf("decode: filelen=%u tlen=%u\n",
	                filelen,tlen) ;
	        }
#endif

	        u_seek(sfd,(offset_t) BLOCKLEN,SEEK_SET) ;

	        rs = SR_OK ;
	        while (tlen < filelen) {

	            rlen = MIN((filelen - tlen),BUFLEN) ;
	            rs = u_read(sfd,buf,rlen) ;
	            len = rs ;
	            if (rs <= 0) break ;

#if	CF_DEBUG
	            if (pip->debuglevel > 1)
	                debugprintf("decode: actually read rlen=%d\n",
				rlen) ;
#endif

	            cksum_accum(&filesum,buf,len) ;

	            rs = u_write(ofd,buf,len) ;
		    if (rs < 0) break ;

	            tlen += len ;

	        } /* end while */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("decode: out while rs=%d\n",rs) ;
#endif

	        if (rs < 0)
	            goto badoutread ;

/* now read the extra message if any */

#if	CF_EXTRAMSG

#if	CF_DEBUG
	        if (DEBUGLEVEL(5))
	            debugprintf("decode: msglen=%u\n",msglen) ;
#endif

	        tlen = 0 ;
	        while (tlen < msglen) {

	            rlen = MIN((msglen - tlen),BUFLEN) ;
	            rs = u_read(sfd,buf,rlen) ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(5))
	                debugprintf("decode: emsg u_read() rs=%d\n",rs) ;
#endif

	            len = rs ;
	            if (rs <= 0) break ;

	            cksum_accum(&msgsum,buf,len) ;

	            if (mfd >= 0) {
	                rs = u_write(mfd,buf,len) ;
		        if (rs < 0) break ;
		    }

	            tlen += len ;

	        } /* end while */

	        if (rs < 0)
	            goto badoutread ;
#endif /* CF_EXTRAMSG */

/* wipe out the stage file */

#if	CF_DEBUG
	        if (pip->debuglevel > 1)
	            debugprintf("decode: wiping output stage file \n") ;
#endif

	        u_seek(sfd,(offset_t) 0,SEEK_SET) ;

	        for (i = 0 ; i < datablocks ; i += 1) {
	            for (j = 0 ; j < n ; j += 1) {
	                randomvar_getulong(rvp,(scrambled + j)) ;
		    }
	            rs = u_write(sfd,scrambled,BLOCKLEN) ;
		    if (rs < 0) break ;
	        } /* end for */

	        u_close(sfd) ;

	    } /* end if (output staged or not) */

#endif /* CF_UNIFIED */

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("decode: finished separating rs=%d\n",rs) ;
#endif

	} /* end block */

/* do the cksums match up? */

#if	CF_UNIFIED

	decode_finish(pip,&ms) ;

	val = ms.main.cksum ;

#else /* CF_UNIFIED */

	rs = SR_OK ;
	rs1 = cksum_getsum(&filesum,&val) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("decode: cksum_getsum() rs=%d\n",rs1) ;
#endif

#endif /* CF_UNIFIED */

#if	CF_DEBUG
	if (pip->debuglevel > 1) {
	    debugprintf("decode: file cksum=\\x%08x (%u)\n",
	        filecksum,filecksum) ;
	    debugprintf("decode: computed cksum=\\x%08x (%u)\n",
	        val,val) ;
	}
#endif

	if (filecksum != val) {

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("decode: checksum didn't match!\n") ;
#endif

	    rs = SR_INVALID ;
	    logfile_printf(&pip->lh,
	        "checksum mismatch, sent=\\x%08x calc=\\x%08x\n",
	        filecksum,val) ;

	}

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	    debugprintf("decode: done normally rs=%d\n",rs) ;
#endif

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("decode: past badoutread rs=%d\n",rs) ;
#endif

#if	(! CF_UNIFIED)
	cksum_finish(&msgsum) ;

	cksum_finish(&filesum) ;
#endif

badfile:
badread:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("decode: past badread\n") ;
#endif

	if (sfd != ofd)
	    u_close(sfd) ;

badtmp2:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("decode: past badtmp2\n") ;
#endif

badoutstat:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("decode: past badoutstat\n") ;
#endif

done:
	if (mfd >= 0)
	    u_close(mfd) ;

retearly:
	if (f_spoolinput)
	    u_close(ifd) ;

badtmp1:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("decode: ret rs=%d\n",rs) ;
#endif

ret0:
	return (rs >= 0) ? filelen : rs ;

/* bad stuff comes here */
badtmpcopy1:
badtmpopen1:
badtmpmk1:
	goto badtmp1 ;

badtmpopen2:
badtmpmk2:
	goto badtmp2 ;

}
/* end subroutine (decode) */


/* local subroutines */


#if	CF_UNIFIED

static int decode_start(pip,msp,l1,l2)
PROGINFO	*pip ;
struct msgstate	*msp ;
int		l1, l2 ;
{
	int		rs ;

	if (pip == NULL) return SR_FAULT ;

	memset(msp,0,sizeof(struct  msgstate)) ;
	msp->main.msglen = l1 ;
	msp->extra.msglen = l2 ;

	rs = cksum_start(&msp->main.sumer) ;
	if (rs < 0)
	    goto bad0 ;

	rs = cksum_start(&msp->extra.sumer) ;
	if (rs < 0)
	    goto bad1 ;

ret0:
	return rs ;

/* bad stuff */
bad1:
	cksum_finish(&msp->main.sumer) ;

bad0:
	goto ret0 ;
}
/* end subroutine (decode_start) */


static int decode_proc(pip,msp,data,datalen)
PROGINFO	*pip ;
struct msgstate	*msp ;
char		data[] ;
int		datalen ;
{
	int		rlen ;
	int		i = 0 ;

	if (pip == NULL) return SR_FAULT ;

	rlen = msp->main.msglen - msp->main.tlen ;
	msp->main.i = 0 ;
	msp->main.wlen = 0 ;
	if ((datalen > 0) && (rlen > 0)) {

	    msp->main.i = i ;
	    msp->main.wlen = MIN(datalen,rlen) ;

	    cksum_accum(&msp->main.sumer,(data + msp->main.i),
	        msp->main.wlen) ;

	    i += msp->main.wlen ;
	    datalen -= msp->main.wlen ;
	    msp->main.tlen += msp->main.wlen ;

	}

	rlen = msp->extra.msglen - msp->extra.tlen ;
	msp->extra.i = 0 ;
	msp->extra.wlen = 0 ;
	if ((datalen > 0) && (rlen > 0)) {

	    msp->extra.i = i ;
	    msp->extra.wlen = MIN(datalen,rlen) ;

	    cksum_accum(&msp->extra.sumer,(data + msp->extra.i),
	        msp->extra.wlen) ;

	    i += msp->extra.wlen ;
	    datalen -= msp->extra.wlen ;
	    msp->extra.tlen += msp->extra.wlen ;

	}

	return datalen ;
}
/* end subroutine (decode_proc) */


static int decode_finish(pip,msp)
PROGINFO	*pip ;
struct msgstate	*msp ;
{
	int		rs = SR_OK ;
	int		rs1 ;
	uint		val ;

	if (pip == NULL) return SR_FAULT ;

	cksum_getsum(&msp->main.sumer,&val) ;

	msp->main.cksum = val ;

	cksum_getsum(&msp->extra.sumer,&val) ;

	msp->extra.cksum = val ;

/* free up and get out */

	rs1 = cksum_finish(&msp->main.sumer) ;
	if (rs >= 0) rs = rs1 ;

	rs1 = cksum_finish(&msp->extra.sumer) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (decode_finish) */

#endif /* CF_UNIFIED */



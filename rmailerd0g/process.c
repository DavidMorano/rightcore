/* process */

/* mail this message off to its recipients */
/* last modified %G% version %I% */


#define	CF_DEBUG	1		/* run-time debug print-outs */
#define	CF_VARVE	1		/* ? */


/* revision history:

	= 1994-06-01, David A­D­ Morano

	This program was originally written.


	= 1998-07-01, David A­D­ Morano

	I added the ability to specify the "envelope_from"
	for the case when we add an envelope header to the message.


*/

/* Copyright © 1994,1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine processes the input and mails off the
	message to each of the recipients.

	The format of the transmitted email message is:

	- originating host (removed by our caller and passed to us)

	- job ID
	- envelope "from"
	- message length in bytes
	- recipients (as many as there are, one per line)
	- blank line marking the end-of-header
	- message part (length given by value above)

	This whole sequence may be repeated indefinitely but we
	only process one of these at a time here.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<estrings.h>
#include	<cksum.h>
#include	<pcsconf.h>
#include	<localmisc.h>

#include	"address.h"
#include	"recipient.h"
#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	TIMEBUFLEN
#define	TIMEBUFLEN	80
#endif

#define	O_FLAGS		(O_CREAT | O_RDWR | O_TRUNC)


/* external subroutines */

extern int	sfshrink(const char *,int,const char **) ;
extern int	nextfield(const char *,int,const char **) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	addressparse(), addressjoin() ;
extern int	mktmpfile(char *,mode_t,const char *) ;

extern int	getcname(const char *,char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	debugprinthex(const char *,int,const char *,int) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*timestr_logz(time_t,char *) ;
extern char	*timestr_edate(time_t,char *) ;

#if	CF_DEBUG
extern char	*d_procmode() ;
#endif


/* external variables */

extern int	mailenter(struct proginfo *,struct prog_params *,
			int,int,offset_t,int,bfile *,int,int,int) ;


/* local structures */


/* forward references */

static int	get_efrom() ;
static int	isindomain(), isinlocalmaildomain() ;

static void	longline() ;


/* local variables */


/* exported subroutines */


int process(pip,v,ifp,elp,pn,th,f_seekable,fsize,ofd,efd)
struct proginfo	*pip ;
int		v ;
bfile		*ifp ;
vecstr		*elp ;
int		pn ;
char		th[] ;
int		f_seekable ;
offset_t	fsize ;
int		ofd, efd ;
{
	struct prog_params	p ;

	RECIPIENT	recips ;
	RECIPIENT_HCUR	hc ;
	RECIPIENT_VCUR	vc ;

#if	CF_VARVE
	RECIPIENT_VAL	*ve = NULL ;
#endif

	CKSUM		cs ;
	offset_t	offset ;
	time_t		daytime ;
	uint	cksum, cksum2 ;

	int	rs = SR_OK ;
	int	rs1 ;
	int	mlen, tlen, rlen, len ;
	int	sl, slen ;
	int	cl ;
	int	type, n, nh, hn ;
	int	fd, ifd ;
	int	mode ;			/* SENDMAIL input mode */
	int	msize ;
	int	f_single ;		/* single mailer invocation required */
	int	f_cksum = FALSE, f_cksum2 = FALSE ;
	int	f_error = FALSE ;
	int	f_errlen = FALSE ;
	int	f_errsum = FALSE ;

	const char	*hp, *cp ;

	char	buf[BUFLEN + 1] ;
	char	hostpart[HOSTPARTLEN + 1], localpart[LOCALPARTLEN + 1] ;
	char	ifnamebuf[MAXPATHLEN + 1] ;
	char	envelope_from[HOSTPARTLEN + LOCALPARTLEN + 3] ;
	char	envelope_host[HOSTPARTLEN + 1] ;
	char	options[MAXHOSTNAMELEN + 1] ;
	char	timebuf[TIMEBUFLEN + 1] ;


#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("process: entered\n") ;
#endif

/* initialization */

#if	CF_VARVE
	msize = pip->nrecips * sizeof(RECIPIENT_VAL) ;
	rs = uc_malloc(msize,&ve) ;
	if (rs < 0)
	    goto bad0 ;
#endif /* CF_VARVE */

/* read the job ID from the input */

	rs = breadline(ifp,buf,BUFLEN) ;
	len = rs ;
	if (rs < 0)
	    goto bad1 ;

	if (len > 1) {

	    cl = sfshrink(buf,len,&cp) ;
	    logfile_printf(&pip->lh,"jobid=%t\n",cp,cl) ;

	}

/* get the envelope "from" address */

	get_efrom(pip,ifp,envelope_host,envelope_from) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 4) {
	    debugprintf("process: get_efrom() env_host=%s envfrom=%s\n",
	        envelope_host,envelope_from) ;
	}
#endif

/* message options */

	options[0] = '\0' ;
	rs = breadline(ifp,buf,BUFLEN) ;
	len = rs ;
	if (rs < 0)
	    goto bad1 ;

	if (len > 1) {

	    slen = sfshrink(buf,len,&cp) ;
	    if (slen > 0)
	        strwcpy(options,cp,slen) ;

	}

/* get the envelope message length */

	mlen = -1 ;
	rs = breadline(ifp,buf,BUFLEN) ;
	len = rs ;
	if (rs < 0)
	    goto bad1 ;

	if (len > 1) {

	    cp = NULL ;
	    if ((sl = nextfield(buf,len,&cp)) > 0) {

	        if (cfdeci(cp,sl,&mlen) < 0)
	            mlen = -1 ;

	    }

	    if ((sl > 0) && (cp != NULL) &&
	        ((sl = nextfield(cp + sl,buf + len - cp,&cp)) > 0)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process: field cksum=>%W<\n",cp,sl) ;
#endif

	        f_cksum = (cfdecui(cp,sl,&cksum) >= 0) ;

	    }

	} /* end if (message length) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	    debugprintf("process: mlen=%d f_cksum=%d cksum=\\x%08X\n",
	        mlen,f_cksum,cksum) ;
#endif

/* get the recipients */

	rs = recipient_start(&recips,10) ;
	if (rs < 0)
	    goto bad1 ;

	    while ((rs = breadline(ifp,buf,BUFLEN)) > 1) {
	        len = rs ;

	        if (buf[len - 1] == '\n') {

	            buf[--len] = '\0' ;
	            if ((cl = sfshrink(buf,len,&cp)) > 0) {

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("process: recipient=%t\n",cp,cl) ;
#endif

	                type = addressparse(cp,cl,hostpart,localpart) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("process: type=%d\n",type) ;
#endif

	                rs1 = recipient_already(&recips,
				hostpart,localpart,type) ;

	                if (rs1 == SR_NOTFOUND)
	                    rs = recipient_add(&recips,
				hostpart,localpart,type) ;

#if	CF_DEBUG
	                if (DEBUGLEVEL(2))
	                    debugprintf("process: recipient added\n") ;
#endif

	            }

	        } else
	            longline(pip,ifp,buf,len) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process: bottom while\n") ;
#endif

	    } /* end while (reading recipients) */

	if (rs < 0)
	    goto bad2 ;

/* OK, we have the recipients, how many are there ? */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: about to count hosts\n") ;
#endif

	nh = recipient_counthosts(&recips) ;

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: hosts=%d\n",nh) ;
#endif

	n = recipient_count(&recips) ;

#if	CF_DEBUG
	if (pip->debuglevel >= 4)
	    debugprintf("process: recipients=%d\n",n) ;
#endif

	logfile_printf(&pip->lh,"recipients %d\n",n) ;

	f_single = ((nh == 1) && (n <= NSEND)) ;

/* parameters for use later (maybe setting up the mailer -- SENDMAIL) */

	memset(&p,0,sizeof(struct prog_params)) ;

	p.protocol = PROTONAME ;
	p.transport_host = th ;
	p.envelope_host = envelope_host ;
	p.envelope_from = envelope_from ;
#if	CF_VARVE
	p.rp = ve ;
#endif
	p.ofd = ofd ;
	p.efd = efd ;

#if	CF_DEBUG
	if (pip->debuglevel >= 4) {
	    debugprintf("process: protocol=%s\n",p.protocol) ;
	    debugprintf("process: trans_host=%s\n",p.transport_host) ;
	    debugprintf("process: env_host=%s\n",p.envelope_host) ;
	    debugprintf("process: env_from=%s\n",p.envelope_from) ;
	}
#endif /* CF_DEBUG */

/* try to figure out what mode to use to pop the message to the mailer */

	mode = IM_SEEK ;

#ifdef	COMMENT
	if (f_single && (! (f_seekable && (mlen < 0)))) {
	    mode = IM_PIPE ;

	} else if ((! f_single) && (! (f_seekable && (mlen < 0))))
	    mode = IM_FILE ;

#else /* COMMENT */

	if (! (f_seekable && (mlen < 0)))
	    mode = (f_single && (mlen < 0)) ? IM_PIPE : IM_FILE ;

#endif /* COMMENT */

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("process: mode=%d (%s) mlen=%d\n",
	        mode,d_procmode(mode,timebuf,TIMEBUFLEN),mlen) ;
#endif

/* preliminary work for the different input modes */

	rs = cksum_start(&cs) ;
	if (rs < 0)
	    goto bad2 ;

	offset = -1 ;
	if (mode == IM_FILE) {

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	        debugprintf("process: mode=FILE\n") ;
#endif

	    rs = mktmpfile( ifnamebuf, 0600, pip->xfname) ;
	    if (rs < 0) {
	        logfile_printf(&pip->lh,
	            "could not open an output file, rs=%d\n",rs) ;
	        goto bad3 ;
	    }

	    rs = u_open(ifnamebuf,O_FLAGS,0666) ;
	    ifd = rs ;
	    {
	         u_unlink(ifnamebuf) ;
	         ifnamebuf[0] = '\0' ;
	    }
	    if (rs < 0)
	        goto bad4 ;

	    if (mlen < 0) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process: no mlen copy\n") ;
#endif

	        tlen = 0 ;
	        while ((rs = bread(ifp,buf,BUFLEN)) > 0) {

	            len = rs ;
	            cksum_accum(&cs,buf,len) ;

	            tlen += len ;
	            rs = uc_writen(ifd,buf,len) ;
	            if (rs < 0)
	                break ;

	        } /* end while (reading input) */

	        mlen = tlen ;
	        f_cksum2 = TRUE ;
	        cksum_getsum(&cs,&cksum2) ;

	    } else {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	            debugprintf("process: an mlen copy, mlen=%d\n",mlen) ;
#endif

	        tlen = 0 ;
	        while ((tlen < mlen) && 
	            ((rs = bread(ifp,buf,MIN((mlen - tlen),BUFLEN))) > 0)) {

	            len = rs ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	                debugprintf("process: read len=%d\n",len) ;
#endif

	            cksum_accum(&cs,buf,len) ;

	            tlen += len ;
	            rs = uc_writen(ifd,buf,len) ;
	            if (rs < 0)
	                break ;

	        } /* end while (reading input) */

	        f_cksum2 = TRUE ;
	        cksum_getsum(&cs,&cksum2) ;

	        if (tlen == mlen) {

	            if (f_cksum && (cksum2 != cksum)) {

	                f_error = f_errsum = TRUE ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	                    debugprintf("process: cksum mismatch "
				"ck1=\\x%08x ck2=\\x%08x\n",
	                        cksum,cksum2) ;
#endif

	            }

	        } else {

	            f_error = f_errlen = TRUE ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	                debugprintf("process: mismatch len1=%d len2=%d\n",
	                    mlen,tlen) ;
#endif

	        }

	    } /* end if (mlen) */

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	        debugprintf("process: copied %d\n",tlen) ;
#endif

	} else if (mode == IM_SEEK) {
	    struct ustat	sb ;

#if	CF_DEBUG
	    offset_t	offset2 ;
#endif


#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	        debugprintf("process: mode=SEEK\n") ;
#endif

	    btell(ifp,&offset) ;

	    bcontrol(ifp,BC_FD,&fd) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	        rs = u_tell(fd,&offset2) ;
	        debugprintf("process: btell_off=%lld tell_rs=%d \n",
	            offset,rs) ;
	        debugprintf("process: tell_off=%lld\n", 
	            offset2) ;
	    }
#endif /* CF_DEBUG */

	    ifd = dup(fd) ;

	    bcontrol(ifp,BC_STAT,&sb) ;

#if	CF_DEBUG
	    if (DEBUGLEVEL(2))
	        debugprintf("process: fsize=%ld bcontrol size=%ld\n", 
	            fsize,sb.st_size) ;
#endif

	    if (mlen >= 0) {

	        tlen = sb.st_size - offset ;
	        if (mlen == (sb.st_size - offset)) {

/* perform a cksum operation here */

	            tlen = 0 ;
	            while ((rs >= 0) && (tlen < mlen)) {

	                rlen = MIN((mlen - tlen),BUFLEN) ;
	                rs = bread(ifp,buf,rlen) ;
			len = rs ;
	                if (rs <= 0)
	                    break ;

	                cksum_accum(&cs,buf,len) ;

	                tlen += len ;

	            } /* end while */

	            f_cksum2 = TRUE ;
	            cksum_getsum(&cs,&cksum2) ;

	            if (tlen == mlen) {

	                if (f_cksum && (cksum2 != cksum))
	                    f_error = f_errsum = TRUE ;

	            } else
	                f_error = f_errlen = TRUE ;

	            u_seek(ifd,offset,SEEK_SET) ;

	        } else
	            f_error = f_errlen = TRUE ;

	    } else
	        mlen = sb.st_size - offset ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	        debugprintf("process: mode=SEEK offset=%ld mlen=%d\n",
	            offset,mlen) ;
#endif

	} /* end if (input mode initialization) */

/* process the addresses out as we have them so far */

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	    debugprintf("process: transport_host=%s\n",th) ;
	    debugprintf("process: from=%s\n",envelope_from) ;
	    debugprintf("process: message f_error=%d length=%d\n",
	        f_error,mlen) ;
	}
#endif /* CF_DEBUG */

	if ((rs >= 0) && f_error)
	    goto done ;

	hn = 0 ;
	if (rs >= 0) {

	recipient_hcurbegin(&recips,&hc) ;

	while (recipient_enumhost(&recips,&hc,&hp) >= 0) {
	    RECIPIENT_VAL		*vep ;

#if	(CF_VARVE == 0)
	    RECIPIENT_VAL		ve[NSEND + 1] ;
#endif

	    int	i, j ;


#if	(CF_VARVE == 0)
	    p.rp = ve ;
#endif
	    if (hp == NULL)
	        continue ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	        debugprintf("process: host> %s\n",hp) ;
#endif

	    i = 0 ;
	    recipient_vcurbegin(&recips,&vc) ;

	    while (recipient_fetchvalue(&recips,hp,&vc,&vep) >= 0) {

#if	CF_DEBUG && 0
	        if (DEBUGLEVEL(2))
	            debugprintf("process: value> t=%d h=%s l=%s\n",
	                vep->type,vep->hostpart,vep->localpart) ;
#endif

	        memcpy(ve + i,vep,sizeof(RECIPIENT_VAL)) ;

	        i += 1 ;
	        if (i >= NSEND) {

#if	CF_DEBUG
	            if (pip->debuglevel >= 4) {
	                for (j = 0 ; j < i ; j += 1)
	                    debugprintf("process: 2 value> t=%d h=%s l=%s\n",
	                        ve[j].type,ve[j].hostpart,ve[j].localpart) ;
	            }
#endif /* CF_DEBUG */

	            for (j = 0 ; j < i ; j += 1) {

	                logfile_printf(&pip->lh,"%3d:%3d %s + %s\n",
	                    pn,hn,hp,ve[j].localpart) ;

	            }

#if	CF_DEBUG
	            if (pip->debuglevel >= 4)
	                debugprintf("process: mailenter 1, pp=%p\n",&p) ;
#endif

	            rs = mailenter(pip,&p,i,mode,offset,mlen,ifp,ifd,ofd,efd) ;

	            i = 0 ;

	        } /* end if */

		if (rs < 0) break ;
	    } /* end while (inner) */

	    recipient_vcurend(&recips,&vc) ;

	    if ((rs >= 0) && (i > 0)) {

#if	CF_DEBUG
	        if (DEBUGLEVEL(2)) {
	            for (j = 0 ; j < i ; j += 1)
	                debugprintf("process: 3 value> t=%d h=%s l=%s\n",
	                    ve[j].type,ve[j].hostpart,ve[j].localpart) ;
		}
#endif

	        for (j = 0 ; j < i ; j += 1) {

	            logfile_printf(&pip->lh,"%3d:%3d %s + %s\n",
	                pn,hn,hp,ve[j].localpart) ;

	        }

#if	CF_DEBUG
	        if (pip->debuglevel >= 4)
	            debugprintf("process: mailenter 2, pp=%p\n",&p) ;
#endif

	        rs = mailenter(pip,&p,i,mode,offset,mlen,ifp,ifd,ofd,efd) ;

	    } /* end if (leftovers) */
	    hn += 1 ;

	    if (rs < 0) break ;
	} /* end while (outer) */

	recipient_hcurend(&recips,&hc) ;
	} /* end if */

/* we're done, finish up */
done:
	if (mode == IM_SEEK)
	    bseek(ifp,(offset + mlen),SEEK_SET) ;

#if	CF_DEBUG
	        if (DEBUGLEVEL(2))
	    debugprintf("process: ending offset=%ld\n",(offset + mlen)) ;
#endif

	if (mode == IM_PIPE)
	    mlen = rs ;

	daytime = time(NULL) ;

	if (! f_error) {

	    if (! f_cksum) {

	        if (f_cksum2) {
	            logfile_printf(&pip->lh,
	                "%s message %3d cksum=\\x%08x len=%d\n",
	                timestr_logz(daytime,timebuf),pn,
	                cksum2,mlen) ;

	        } else
	            logfile_printf(&pip->lh,
	                "%s message %3d len=%d\n",
	                timestr_logz(daytime,timebuf),pn,
	                mlen) ;

	    } else
	        logfile_printf(&pip->lh,
	            "%s message %3d cksum=\\x%08x len=%d\n",
	            timestr_logz(daytime,timebuf),pn,
	            cksum2,mlen) ;

	} else {

	    logfile_printf(&pip->lh,
	        "%s message %3d ERROR\n",
	        timestr_logz(daytime,timebuf),pn) ;

	    if (f_errlen) {

	        bprintf(pip->efp,"%s: length mismatch in message=%d\n",
	            pip->progname,pn) ;

	        bprintf(pip->efp,"%s: expected=%d got=%d\n",
	            pip->progname,mlen,tlen) ;

	        logfile_printf(&pip->lh,
	            "length mismatch, expected=%d got=%d\n",
	            mlen,tlen) ;

	    } else if (f_errsum) {

	        bprintf(pip->efp,"%s: cksum mismatch in message=%d\n",
	            pip->progname,pn) ;

	        bprintf(pip->efp,"%s: expected=\\x%08x got=\\x%08x\n",
	            pip->progname,cksum,cksum2) ;

	        logfile_printf(&pip->lh,
	            "cksum mismatch, expected=\\x%08x got=\\x%08x\n",
	            cksum,cksum2) ;

	    }

	}

	if (rs >= 0)
	    rs = (! f_error) ? SR_OK : SR_NOANODE ;

/* bad things happen */
bad5:
	if ((mode == IM_FILE) || (mode == IM_SEEK))
	    u_close(ifd) ;

bad4:
	if ((mode == IM_FILE) && (ifnamebuf[0] != '\0'))
	    u_unlink(ifnamebuf) ;

bad3:
	cksum_finish(&cs) ;

bad2:
	recipient_finish(&recips) ;

/* memory free-ups */
bad1:

#if	CF_VARVE
	if (ve != NULL) {
	    uc_free(ve) ;
	    ve = NULL ;
	}
#endif

bad0:
ret0:
	return rs ;
}
/* end subroutine (process) */


/* local subroutines */


/* waste the rest of a long line */
static void longline(pip,ifp,buf,len)
struct proginfo	*pip ;
bfile		*ifp ;
char		buf[] ;
int		len ;
{


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("longline: entered\n") ;
#endif

	while (((len = breadline(ifp,buf,BUFLEN)) > 0) &&
	    (buf[len - 1] != '\n')) ;

	if (! pip->f.quiet)
	    bprintf(pip->efp,"%s: address too long \"%W\"\n",
	        pip->progname,buf,MIN(len,40)) ;

}
/* end subroutine (longline) */


/* get the envelope "from" address */
static int get_efrom(pip,fp,envelope_host,envelope_from)
struct proginfo	*pip ;
bfile		*fp ;
char		envelope_host[] ;
char		envelope_from[] ;
{
	int	rs = SR_OK ;
	int	len ;
	int	cl ;

	const char	*cp ;

	char	linebuf[LINELEN + 1] ;
	char	localpart[LOCALPARTLEN + 1] ;


	envelope_from[0] = '\0' ;
	envelope_host[0] = '\0' ;
	if ((rs = breadline(fp,linebuf,LINELEN)) > 1) {
	    len = rs ;

	    if ((cl = sfshrink(linebuf,len,&cp)) > 0) {

#if	CF_DEBUG
	        if (pip->debuglevel >= 4)
	            debugprintf("get_efrom: efrom=%r\n",cp,cl) ;
#endif

	        if (addressparse(cp,cl,envelope_host,localpart) < 0)
	            envelope_host[0] = '\0' ;

#if	CF_DEBUG
	        if (pip->debuglevel >= 4)
	            debugprintf("get_efrom: ehost=%s\n",envelope_host) ;
#endif

	        if (((envelope_host[0] != '\0') &&
	            (! isinlocalmaildomain(pip,envelope_host))) ||
	            (strcmp(localpart,pip->username) != 0)) {

	            strwcpy(envelope_from,cp,cl) ;

	            logfile_printf(&pip->lh,
	                "from=%s\n",envelope_from) ;

	        } else
	            logfile_printf(&pip->lh,"from=%s!%s\n",
	                pip->domainname,pip->username) ;

	    }

	} /* end if */

	return rs ;
}
/* end subroutine (get_efrom) */


/* is the given domain really in the current mail domain ? */
static int isinlocalmaildomain(pip,name)
struct proginfo	*pip ;
const char	name[] ;
{
	char	*dotp ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("isinlocalmaildomain: entered name=%s\n",
	        name) ;
#endif

	if (isindomain(pip,pip->maildomain,name))
	    return TRUE ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("isinlocalmaildomain: not in our domain !\n") ;
#endif

	if (((dotp = strchr(name,'.')) != NULL) &&
	    isindomain(pip,pip->maildomain,(dotp + 1)))
	    return TRUE ;

#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("isinlocalmaildomain: not in our maildomain !\n") ;
#endif

	return FALSE ;
}
/* end subroutine (isinlocalmaildomain) */


/* is the named host in the domain given ? */
static int isindomain(pip,domainname,name)
struct proginfo	*pip ;
const char	domainname[], name[] ;
{
	int	f_unqualled = TRUE ;
	int	i ;

	char	namebuf[MAXHOSTNAMELEN + 1] ;
	char	*dotp ;


#if	CF_DEBUG
	if (pip->debuglevel > 1)
	    debugprintf("isindomain: d=%s name=%s\n",
	        domainname,name) ;
#endif

	if ((dotp = strchr(name,'.')) != NULL)
	    f_unqualled = FALSE ;

	if (f_unqualled) {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("isindomain: unqualled\n") ;
#endif

	    if (strcmp(name,pip->nodename) == 0)
	        return TRUE ;

	    bufprintf(namebuf,MAXHOSTNAMELEN,"%s.%s",
	        name,domainname) ;

	    if (getcname(namebuf,NULL) >= 0)
	        return TRUE ;

	} else {

#if	CF_DEBUG
	    if (pip->debuglevel > 1)
	        debugprintf("isindomain: qualled d=%s n=%s\n",
	            domainname,dotp) ;
#endif

	    dotp += 1 ;
	    if (((i = substring(name,-1,domainname)) >= 0) &&
	        (name[i + strlen(domainname)] == '\0'))
	        return TRUE ;

	}

	return FALSE ;
}
/* end subroutine (isindomain) */




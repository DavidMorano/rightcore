/* mfs-cmd */

/* client-side code to initiate commands to the server */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_DEBUG	0		/* switchable debug print-outs */


/* revision history:

	= 2008-09-01, David A­D­ Morano
        This subroutine was adopted from the DWD program. I may not have changed
        all of the comments correctly though!

	= 2017-08-10, David A­D­ Morano
	This subroutine was borrowed to code MFSERVE.

*/

/* Copyright © 2008,2017 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        These subroutines handle some IPC related functions. This is client-side
        code which interacts with the server code (within the server (MFSERVE).


*******************************************************************************/


#if	defined(SFIO) && (SFIO > 0)
#define	CF_SFIO	1
#else
#define	CF_SFIO	0
#endif

#if	(defined(KSHBUILTIN) && (KSHBUILTIN > 0))
#include	<shell.h>
#endif

#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/socket.h>
#include	<netinet/in.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"shio.h"
#include	"mfsmsg.h"
#include	"mfsmain.h"
#include	"mfsc.h"
#include	"mfslocinfo.h"
#include	"mfslog.h"
#include	"defs.h"


/* local defines */


/* external subroutines */

extern int	snddd(char *,int,uint,uint) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmpusd(const char *,int,mode_t,char *) ;
extern int	isNotPresent(int) ;
extern int	isBadSend(int) ;

extern int	progtmpdir(PROGINFO *,char *) ;
extern int	progcheckdir(PROGINFO *,char *,int,int) ;
extern int	progreqfile(PROGINFO *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*getourenv(cchar **,cchar *) ;
extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnrpbrk(cchar *,int,cchar *) ;
extern char	*timestr_logz(time_t,char *) ;


/* external variables */


/* local structures */


/* forward references */

static int	mfscmder(PROGINFO *,MFSC *,SHIO *) ;

static int	mfscmd_status(PROGINFO *,MFSC *,SHIO *) ;
static int	mfscmd_help(PROGINFO *,MFSC *,SHIO *) ;
static int	mfscmd_mark(PROGINFO *,MFSC *,SHIO *) ;
static int	mfscmd_exit(PROGINFO *,MFSC *,SHIO *) ;
static int	mfscmd_listeners(PROGINFO *,MFSC *,SHIO *) ;


/* local variables */

static cchar	*cmds[] = {
	"status",
	"help",
	"getval",
	"mark",
	"exit",
	"listeners",
	NULL
} ;

enum cmds {
	cmd_status,
	cmd_help,
	cmd_getval,
	cmd_mark,
	cmd_exit,
	cmd_listeners,
	cmd_overlast
} ;


/* exported subroutines */


int mfscmd(PROGINFO *pip,cchar *ofn)
{
	LOCINFO		*lip = pip->lip ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;

	if ((rs = locinfo_cmdscount(lip)) > 0) {
	    SHIO	ofile, *ofp = &ofile ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;

	    if ((ofn == NULL) || (ofn[0] == '\0'))
	        ofn = STDOUTFNAME ;

	    if ((rs = shio_open(ofp,ofn,"wct",0666)) >= 0) {
		MFSC		c ;
		const int	to = TO_OPENSERVE ;
		cchar		*pr = pip->pr ;
		if ((rs = mfsc_open(&c,pr,to)) > 0) {
		    {
			rs = mfscmder(pip,&c,ofp) ;
			wlen += rs ;
		    }
	    	    rs1 = mfsc_close(&c) ;
	    	    if (rs >= 0) rs = rs1 ;
		} else if (rs == 0) {
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("mfscmd: SN rs=%d\n",rs) ;
#endif
	    	    fmt = "%s: server not available (%d)\n" ;
	    	    shio_printf(pip->efp,fmt,pn,rs) ;
		} else if (isNotPresent(rs)) {
#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("mfscmd: SF rs=%d\n",rs) ;
#endif
	    	    fmt = "%s: server failure (%d)\n" ;
	    	    shio_printf(pip->efp,fmt,pn,rs) ;
		} /* end if (mfsc) */
		rs1 = shio_close(ofp) ;
		if (rs >= 0) rs = rs1 ;
	   } else {
		fmt = "%s: inaccessible output (%d)\n" ;
		shio_printf(pip->efp,fmt,pn,rs) ;
		shio_printf(pip->efp,"%s: ofile=%s\n",pn,ofn) ;
	   } /* end if (file-output) */
	} /* end if (positive) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	debugprintf("mfscmd: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mfscmd) */


int mfscmd_svcname(PROGINFO *pip,int idx,cchar **rpp)
{
	const int	nidx = nelem(cmds) ;
	int		rs = SR_OK ;
	int		len = 0 ;

	if (pip == NULL) return SR_FAULT ;

	if ((idx >= 0) && (idx < nidx)) {
	    if (cmds[idx] != NULL) {
	        len = strlen(cmds[idx]) ;
	    }
	} else {
	    rs = SR_NOTFOUND ;
	}

	if (rpp != NULL) {
	    *rpp = (rs >= 0) ? cmds[idx] : NULL ;
	}

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (mfscmd_svcname) */


/* local subroutines */


static int mfscmder(PROGINFO *pip,MFSC *pcp,SHIO *ofp)
{
	LOCINFO		*lip = pip->lip ;
	KEYOPT		*kop ;
	KEYOPT_CUR	kcur ;
	int		rs ;
	int		c = 0 ;

	kop = &lip->cmds ;
	if ((rs = keyopt_curbegin(kop,&kcur)) >= 0) {
	    int		ci ;
	    int		kl ;
	    cchar	*kp ;
	    cchar	*pn = pip->progname ;
	    cchar	*fmt ;

	    while (rs >= 0) {
	        kl = keyopt_enumkeys(kop,&kcur,&kp) ;
	        if (kl == SR_NOTFOUND) break ;
	        rs = kl ;

	        if (rs > 0) {
	            if ((ci = matostr(cmds,1,kp,kl)) >= 0) {
	                c += 1 ;
	                switch (ci) {
	                case cmd_status:
	                    rs = mfscmd_status(pip,pcp,ofp) ;
	                    break ;
	                case cmd_help:
	                    rs = mfscmd_help(pip,pcp,ofp) ;
	                    break ;
	                case cmd_mark:
	                    rs = mfscmd_mark(pip,pcp,ofp) ;
	                    break ;
	                case cmd_exit:
	                    rs = mfscmd_exit(pip,pcp,ofp) ;
	                    break ;
	                case cmd_listeners:
	                    rs = mfscmd_listeners(pip,pcp,ofp) ;
	                    break ;
	                } /* end switch */
		    } else {
	                    fmt = "%s: unknown cmd=%t\n" ;
	                    shio_printf(pip->efp,fmt,pn,kp,kl) ;
			    {
				char	tbuf[TIMEBUFLEN+1] ;
				timestr_logz(pip->daytime,tbuf) ;
				logprintf(pip,"%s unknown cmd=%t",tbuf,kp,kl) ;
			    }
	            } /* end if (valid) */
	        } /* end if (positive) */

	    } /* end while */

	    keyopt_curend(kop,&kcur) ;
	} /* end if (keyopt-cur) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (proccmd) */


static int mfscmd_status(PROGINFO *pip,MFSC *pcp,SHIO *ofp)
{
	MFSC_STATUS	stat ;
	int		rs ;
	int		wlen = 0 ;
	cchar		*fmt ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfscmd_status: ent\n") ;
#endif

	if (pip == NULL) return SR_FAULT ;

	if ((rs = mfsc_status(pcp,&stat)) >= 0) {
	    fmt = "server rc=%u pid=%u queries=%u\n" ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("mfscmd_status: mfsc_status() rs=%d\n",rs) ;
#endif
	    rs = shio_printf(ofp,fmt,rs,stat.pid,stat.queries) ;
	    wlen += rs ;
	} else if (isBadSend(rs)) {
	    fmt = "server failure (%d)\n" ;
#if	CF_DEBUG
	    if (DEBUGLEVEL(4))
	        debugprintf("mfscmd_status: mfsc_status() rs=%d\n",rs) ;
#endif
	    rs = shio_printf(ofp,fmt,rs) ;
	    wlen += rs ;
	} /* end if (mfsc_status) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("mfscmd_status: ret rs=%d wlen=%u\n",rs,wlen) ;
#endif

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mfscmd_status) */


static int mfscmd_help(PROGINFO *pip,MFSC *pcp,SHIO *ofp)
{
	const int	rlen = MAXNAMELEN ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	char		*rbuf ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = uc_malloc((rlen+1),&rbuf)) >= 0) {
	    int		i ;
	    for (i = 0 ; rs >= 0 ; i += 1) {
	        if ((rs = mfsc_help(pcp,rbuf,rlen,i)) > 0) {
		    rs = shio_print(ofp,rbuf,rs) ;
		    wlen += rs ;
		}
		if (rs == 0) break ;
	    } /* end for */
	    rs1 = uc_free(rbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (m-a-f) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mfscmd_help) */


static int mfscmd_mark(PROGINFO *pip,MFSC *pcp,SHIO *ofp)
{
	int		rs ;
	int		wlen = 0 ;
	cchar		*fmt ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = mfsc_mark(pcp)) >= 0) {
	    fmt = "marking (%d)\n" ;
	    rs = shio_printf(ofp,fmt,rs) ;
	    wlen += rs ;
	} else if (isBadSend(rs)) {
	    fmt = "server failure (%d)\n" ;
	    rs = shio_printf(ofp,fmt,rs) ;
	    wlen += rs ;
	} /* end if (mfsc_mark) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mfscmd_mark) */


static int mfscmd_exit(PROGINFO *pip,MFSC *pcp,SHIO *ofp)
{
	int		rs ;
	int		wlen = 0 ;
	cchar		*fmt ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = mfsc_exit(pcp,"extern")) >= 0) {
	    fmt = "exiting (%d)\n" ;
	    rs = shio_printf(ofp,fmt,rs) ;
	    wlen += rs ;
	} else if (isBadSend(rs)) {
	    fmt = "server failure (%d)\n" ;
	    rs = shio_printf(ofp,fmt,rs) ;
	    wlen += rs ;
	} /* end if (mfsc_exit) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mfscmd_exit) */


static int mfscmd_listeners(PROGINFO *pip,MFSC *pcp,SHIO *ofp)
{
	const int	rlen = (2*MAXPATHLEN) ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	char		*rbuf ;

	if (pip == NULL) return SR_FAULT ;

	if ((rs = uc_malloc((rlen+1),&rbuf)) >= 0) {
	    int		i ;
	    for (i = 0 ; rs >= 0 ; i += 1) {
	        if ((rs = mfsc_listener(pcp,rbuf,rlen,i)) > 0) {
		    rs = shio_print(ofp,rbuf,rs) ;
		    wlen += rs ;
		}
		if (rs == 0) break ;
	    } /* end for */
	    rs1 = uc_free(rbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (m-a-f) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (mfscmd_listeners) */



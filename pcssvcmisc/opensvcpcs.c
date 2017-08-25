/* opensvcpcs */

/* PCS facility service (name) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2003-11-04, David A­D­ Morano
	This code was started by taking the corresponding code from the
	TCP-family module.  In retrospect, that was a mistake.  Rather I should
	have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	These are (PCS) utility subroutines for use by PCS open-service
	modules.  This is essentially an object also (SUBPCS).


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bits.h>
#include	<field.h>
#include	<sysusernames.h>
#include	<getxusername.h>
#include	<pcsns.h>
#include	<filebuf.h>
#include	<nulstr.h>
#include	<localmisc.h>

#include	"opensvcpcs.h"
#include	"defs.h"


/* local defines */

#define	VARMOTDUSER	"MOTD_USERNAME"
#define	VARMOTDUID	"MOTD_UID"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sfskipwhite(cchar *,int,cchar **) ;
extern int	matstr(cchar **,cchar *,int) ;
extern int	matostr(cchar **,int,cchar *,int) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(cchar *,int) ;
extern int	optvalue(cchar *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	openshmtmp(char *,int,mode_t) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	statvfsdir(const char *,struct statvfs *) ;
extern int	getusername(char *,int,uid_t) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	isdigitlatin(int) ;

#if	CF_DEBUGS
extern int	debugopen(const char *) ;
extern int	debugprintf(const char *,...) ;
extern int	debugclose() ;
extern int	strlinelen(const char *,int,int) ;
extern int	nprintf(const char *,const char *,...) ;
#endif

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	getuser(cchar **,char *,int) ;


/* local variables */

static const uchar	aterms[] = {
	0x00, 0x2E, 0x00, 0x00,
	0x09, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
} ;


/* exported subroutines */


int subpcs_start(SUBPCS *sip,cchar *pr,cchar **ev,int w)
{
	int		rs ;
	memset(sip,0,sizeof(SUBPCS)) ;
	sip->pr = pr ;
	sip->envv = ev ;
	sip->w = w ;
	rs = pcsns_open(&sip->ns,pr) ;
	return rs ;
}
/* end subroutine (subpcs_start) */


int subpcs_finish(SUBPCS *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	rs1 = pcsns_close(&sip->ns) ;
	if (rs >= 0) rs = rs1 ;
	return rs ;
}
/* end subroutine (subpcs_finish) */


int subpcs_all(SUBPCS *sip,FILEBUF *ofp)
{
	SYSUSERNAMES	u ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs = sysusernames_open(&u,NULL)) >= 0) {
	    const int	ulen = USERNAMELEN ;
	    char	ubuf[USERNAMELEN+1] ;
	    while ((rs = sysusernames_readent(&u,ubuf,ulen)) > 0) {
		rs = subpcs_user(sip,ofp,ubuf,rs) ;
		wlen += rs ;
		if (rs < 0) break ;
	    } /* end while (reading entreies) */
	    rs1 = sysusernames_close(&u) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (sysusernames) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subpcs_all) */


/* ARGSUSED */
int subpcs_args(SUBPCS *sip,FILEBUF *ofp,ARGINFO *aip,BITS *bop,
		cchar *afn)
{
	const int	argc = aip->argc ;
	int		rs = SR_OK ;
	int		ai ;
	int		wlen = 0 ;
	int		f ;
	cchar		**argv = aip->argv ;
	cchar		*cp ;
	for (ai = 1 ; ai < argc ; ai += 1) {
	    f = (ai <= aip->ai_max) && (bits_test(bop,ai) > 0) ;
	    f = f || ((ai > aip->ai_pos) && (argv[ai] != NULL)) ;
	    if (f) {
		cp = argv[ai] ;
	        if (cp[0] != '\0') {
	    	    rs = subpcs_user(sip,ofp,cp,-1) ;
	    	    wlen += rs ;
		}
	    }
	} /* end for (handling positional arguments) */
	if (rs >= 0) {
	    rs = subpcs_af(sip,ofp,afn) ;
	    wlen += rs ;
	}
	if ((rs >= 0) && (wlen == 0)) {
	    rs = subpcs_def(sip,ofp) ;
	    wlen += rs ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subpcs_args) */


int subpcs_af(SUBPCS *sip,FILEBUF *ofp,cchar *afn)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		wlen = 0 ;
	if ((rs >= 0) && (afn != NULL) && (afn[0] != '\0')) {
	    const mode_t	om = 0666 ;
	    const int		of = O_RDONLY ;
	    if ((rs = uc_open(afn,of,om)) >= 0) {
		FILEBUF		afile, *afp = &afile ;
		const int	to = -1 ;
		const int	afd = rs ;
		if ((rs = filebuf_start(afp,afd,0L,0,0)) >= 0) {
	            const int	llen = LINEBUFLEN ;
	            int		len ;
		    int		cl ;
		    cchar	*cp ;
	            char	lbuf[LINEBUFLEN + 1] ;

	            while ((rs = filebuf_readline(afp,lbuf,llen,to)) > 0) {
	                len = rs ;

	                if (lbuf[len - 1] == '\n') len -= 1 ;
	                lbuf[len] = '\0' ;

	                if ((cl = sfskipwhite(lbuf,len,&cp)) > 0) {
	                    if (cp[0] != '#') {
				lbuf[((cp-lbuf)+cl)] = '\0' ;
	                        rs = subpcs_users(sip,ofp,cp,cl) ;
				wlen += rs ;
	                    }
	                }

			if (rs < 0) break ;
	            } /* end while (reading lines) */

	            rs1 = filebuf_finish(afp) ;
	    	    if (rs >= 0) rs = rs1 ;
	        }  /* end if (filebuf) */
		u_close(afd) ;
	    } /* end if (file) */
	} /* end if (processing argument-list file) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subpcs_af) */


int subpcs_def(SUBPCS *sip,FILEBUF *ofp)
{
	const int	ulen = USERNAMELEN ;
	int		rs ;
	int		wlen = 0 ;
	cchar		**envv = sip->envv ;
	char		ubuf[USERNAMELEN+1] ;
	if ((rs = getuser(envv,ubuf,ulen)) > 0) {
	    rs = subpcs_user(sip,ofp,ubuf,rs) ;
	    wlen += rs ;
	}
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subpcs_def) */


int subpcs_users(SUBPCS *sip,FILEBUF *ofp,cchar *lbuf,int llen)
{
	FIELD		fsb ;
	int		rs ;
	int		wlen = 0 ;
	if ((rs = field_start(&fsb,lbuf,llen)) >= 0) {
	    int		fl ;
	    cchar	*fp ;
	    while ((fl = field_get(&fsb,aterms,&fp)) >= 0) {
	        if (fl > 0) {
	            rs = subpcs_user(sip,ofp,fp,fl) ;
	            wlen += rs ;
	        }
	        if (fsb.term == '#') break ;
	        if (rs < 0) break ;
	    } /* end while */
	    field_finish(&fsb) ;
	} /* end if (field) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subpcs_users) */


int subpcs_user(SUBPCS *sip,FILEBUF *ofp,cchar *sp,int sl)
{
	NULSTR		n ;
	const int	w = sip->w ;
	int		rs ;
	int		rs1 ;
	int		wlen = 0 ;
	cchar		*un ;
	if ((rs = nulstr_start(&n,sp,sl,&un)) >= 0) {
	    const int	rlen = REALNAMELEN ;
	    char	rbuf[REALNAMELEN+1] ;
	    if ((rs = pcsns_get(&sip->ns,rbuf,rlen,un,w)) >= 0) {
	        rs = filebuf_print(ofp,rbuf,rs) ;
	        wlen += rs ;
	    }
	    rs1 = nulstr_finish(&n) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (nulstr) */
	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (subpcs_user) */


static int getuser(cchar **envv,char *ubuf,int ulen)
{
	int		rs = SR_OK ;
	cchar		*un = getourenv(envv,VARMOTDUSER) ;
	ubuf[0] = '\0' ;
	if ((un == NULL) || (un[0] == '\0')) {
	    cchar	*uidp = getourenv(envv,VARMOTDUID) ;
	    if ((uidp != NULL) && (uidp[0] != '\0')) {
		int	v ;
		if ((rs = cfdeci(uidp,-1,&v)) >= 0) {
		    const uid_t	uid = v ;
		    rs = getusername(ubuf,ulen,uid) ;
		}
	    } else {
		rs = getusername(ubuf,ulen,-1) ;
	    }
	} else {
	    rs = sncpy1(ubuf,ulen,un) ;
	}
	return rs ;
}
/* end subroutine (getuser) */



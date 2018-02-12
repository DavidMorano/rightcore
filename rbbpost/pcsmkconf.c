/* pcsmkconf */

/* make the PCS CONF index file */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	- 2008-10-01, David A­D­ Morano
	This module was originally written.

*/

/* Copyright © 2008 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine makes the PCS CONF index file.


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
#include	<ids.h>
#include	<vecstr.h>
#include	<paramfile.h>
#include	<varmk.h>
#include	<localmisc.h>


/* local defines */

#define	SUBINFO		struct subinfo
#define	SUBINFO_FL	struct subinfo_flags

#define	PRCONF		"conf"

#ifndef	PARAMBUFLEN
#define	PARAMBUFLEN	256
#endif


/* external subroutines */

extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkfnamesuf1(char *,const char *,const char *) ;
extern int	nleadstr(const char *,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	pathclean(char *,const char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	fperm(int,uid_t,gid_t,gid_t *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	vecstr_adduniq(vecstr *,const char *,int) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	vecstr_loadfile(vecstr *,int,const char *) ;

extern int	prmktmpdir(const char *,char *,const char *,mode_t) ;
extern int	mktmpuserdir(char *,const char *,const char *,mode_t) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */

extern char	**environ ;


/* local structures */

struct subinfo_flags {
	uint		localconf:1 ;
} ;

struct subinfo {
	SUBINFO_FL	f ;
	PARAMFILE	pf ;
	VARMK		v ;
	const char	**envv ;
	const char	*prconf ;
	const char	*pr ;
	const char	*cfname ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,cchar *,cchar **,cchar *) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_startsubs(SUBINFO *,VECSTR *) ;
static int	subinfo_confglobal(SUBINFO *,char *) ;
static int	subinfo_conflocal(SUBINFO *,char *) ;
static int	subinfo_proc(SUBINFO *) ;


/* global variables */


/* local variables */

static const char	*schedconf[] = {
	"%p/etc/%n.%f",
	"%p/etc/%f",
	"%p/%n.%f",
	NULL
} ;


/* exported subroutines */


int pcsmkconf(pr,envv,cfname)
const char	pr[] ;
const char	*envv[] ;
const char	cfname[] ;
{
	SUBINFO		si ;
	int		rs ;
	int		rs1 ;

	if (pr == NULL) return SR_FAULT ;

#if	CF_DEBUGS
	debugprintf("pcsmkconf: pr=%s\n",pr) ;
	debugprintf("pcsmkconf: cfname=%s\n",cfname) ;
#endif

	if (pr[0] == '\0')
	    return SR_INVALID ;

	if ((cfname != NULL) && (cfname[0] == '\0')) return SR_INVALID ;

	if ((rs = subinfo_start(&si,pr,envv,cfname)) >= 0) {

	    rs = subinfo_proc(&si) ;

	    rs1 = subinfo_finish(&si) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

	if (rs == SR_EXIST) rs = SR_OK ;

#if	CF_DEBUGS
	debugprintf("pcsmkconf: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (pcsmkconf) */


/* local subroutines */


static int subinfo_start(sip,pr,envv,cfname)
SUBINFO		*sip ;
const char	pr[] ;
const char	*envv[] ;
const char	cfname[] ;
{
	VECSTR		subs ;
	const mode_t	vm = 0444 ;
	const int	of = O_CREAT ;
	const int	n = 20 ;
	const int	f_global = (cfname == NULL) ;
	int		rs = SR_OK ;
	char		dbname[MAXPATHLEN+1] ;

	if (envv == NULL) envv = (const char **) environ ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->envv = envv ;
	sip->prconf = PRCONF ;
	sip->pr = pr ;
	sip->cfname = cfname ;

/* open the PCS-configuration file (if it exists) */

	if ((rs = vecstr_start(&subs,4,0)) >= 0) {
	    char	tmpfname[MAXPATHLEN+1] ;

	    rs = subinfo_startsubs(sip,&subs) ;

	    if ((rs >= 0) && (cfname == NULL)) {
	        mode_t	m = R_OK ;
	        int	tlen = MAXPATHLEN ;
	        char	*tbuf = tmpfname ;
	        cfname = tmpfname ;
	        rs = permsched(schedconf,&subs, tbuf,tlen, sip->prconf,m) ;
	    }

	    vecstr_finish(&subs) ;
	} /* end if (subs) */

	if (rs < 0) goto bad0 ;

	if (f_global) {
	    rs = subinfo_confglobal(sip,dbname) ;
	} else {
	    rs = subinfo_conflocal(sip,dbname) ;
	}
	if (rs < 0) goto bad1 ;

/* see if we can create a new VAR DB */

	rs = varmk_open(&sip->v,dbname,of,vm,n) ;
	if (rs < 0) goto bad1 ; /* can return SR_EXIST */

/* done */
ret0:
	return rs ;

/* bad stuff */
bad1:
bad0:
	goto ret0 ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(sip)
SUBINFO		*sip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	rs1 = varmk_close(&sip->v) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_startsubs(SUBINFO *sip,VECSTR *slp)
{
	int		rs = SR_OK ;
	int		cl ;
	const char	*cp ;

	cl = sfbasename(sip->pr,-1,&cp) ;
	if (cl <= 0) rs = SR_INVALID ;

	if (rs >= 0)
	    rs = vecstr_envadd(slp,"p",sip->pr,-1) ;

	if (rs >= 0)
	    rs = vecstr_envadd(slp,"n",cp,cl) ;

	return rs ;
}
/* end subroutine (subinfo_startsubs) */


static int subinfo_confglobal(SUBINFO *sip,char *dbname)
{
	const mode_t	dm = 0777 ;
	int		rs ;
	const char	*cdname = "pcsconf" ;
	char		tmpdname[MAXPATHLEN+1] ;

	if ((rs = prmktmpdir(sip->pr,tmpdname,cdname,dm)) >= 0) {
	    rs = mkpath2(dbname,tmpdname,sip->prconf) ;
	}

	return rs ;
}
/* end subroutine (subinfo_confglobal) */


static int subinfo_conflocal(SUBINFO *sip,char *dbname)
{
	const mode_t	dm = 0775 ;
	int		rs ;
	const char	*cdname = "pcsconf" ;
	char		tmpdname[MAXPATHLEN+1] ;

	if ((rs = mktmpuserdir(tmpdname,"-",cdname,dm)) >= 0) {
	    rs = mkpath2(dbname,tmpdname,sip->prconf) ;
	}

	return rs ;
}
/* end subroutine (subinfo_conflocal) */


static int subinfo_proc(SUBINFO *sip)
{
	PARAMFILE	*pfp = &sip->pf ;
	PARAMFILE_CUR	cur ;
	PARAMFILE_ENT	pe ;
	int		rs ;
	int		rs1 ;

	if ((rs = paramfile_open(pfp,sip->envv,sip->cfname)) >= 0) {

	    if ((rs = paramfile_curbegin(pfp,&cur)) >= 0) {
	        const int	plen = PARAMBUFLEN ;
	        int		kl ;
	        char		pbuf[PARAMBUFLEN+1] ;

	        while (rs >= 0) {
	            kl = paramfile_enum(pfp,&cur,&pe,pbuf,plen) ;
	            if (kl == SR_NOTFOUND) break ;
	            rs = kl ;

		    if (rs >= 0) {
	                rs = varmk_addvar(&sip->v,pe.key,pe.value,pe.vlen) ;
		    }

	        } /* end while (reading parameters) */

	        paramfile_curend(pfp,&cur) ;
	    } /* end if (cursor) */

	    rs1 = paramfile_close(pfp) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (paramfile) */

	return rs ;
}
/* end subroutine (subinfo_proc) */



/* prgetprogpath */

/* get the path to a program that is used within the PCS system */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUGN	0		/* special debugging */


/* revision history:

	= 1998-11-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine is used to find a PCS related program and to verify
	that it is executable.

	Important:

	This subroutine is different from 'pcsgetprog(3pcs)' in that this will
	return a full path of the found program whenever it is different than
	what was supplied.  In contrast, the 'pcsgetprog(3pcs)' subroutine only
	returns the full path of the found program when it is not absolute and
	it is found in the PCS distribution.

	Synopsis:

	int prgetprogpath(pr,rbuf,np,nl)
	cchar		pr[] ;
	char		rbuf[] ;
	cchar		np[] ;
	int		nl ;

	Arguments:

	pr		PCS program root path
	rbuf		returned file path if not the same as input
	np		name
	nl		name-length

	Returns:

	>0		found the program path and this is the length
	==0		program was found w/o a path prefix
	<0		program was not found

	programpath	returned file path if it was not in the PWD


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<vecstr.h>
#include	<ids.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local typedefs */

#ifndef	TYPEDEF_CCHAR
#define	TYPEDEF_CCHAR	1
typedef cchar	cchar ;
#endif


/* local defines */

#ifndef	VARPATH
#define	VARPATH		"PATH"
#endif

#define	SUBINFO		struct subinfo

#define	NDF		"prgetprogpath.deb"


/* external subroutines */

extern int	sncpy2w(char *,int,cchar *,cchar *,int) ;
extern int	sncpy3w(char *,int,cchar *,cchar *,cchar *,int) ;
extern int	mkpath1(char *,cchar *) ;
extern int	mkpath2(char *,cchar *,cchar *) ;
extern int	mkpath3(char *,cchar *,cchar *,cchar *) ;
extern int	mkpath1w(char *,cchar *,int) ;
extern int	mkpath2w(char *,cchar *,cchar *,int) ;
extern int	pathaddw(char *,int,cchar *,int) ;
extern int	sperm(IDS *,struct ustat *,int) ;
extern int	perm(cchar *,uid_t,gid_t,gid_t *,int) ;
extern int	isNotPresent(int) ;
extern int	isOneOf(const int *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(cchar *,...) ;
extern int	strnnlen(cchar *,int,int) ;
#endif
#if	CF_DEBUGN
extern int	nprintf(cchar *,cchar *,...) ;
#endif

extern char	*strwcpy(char *,cchar *,int) ;
extern char	*strnchr(cchar *,int,int) ;


/* external variables */


/* local structures */

struct subinfo {
	cchar		*pr ;
	IDS		id ;
	vecstr		dirs ;
	uint		f_dirs:1 ;
	uint		f_changed:1 ;
	uint		f_done:1 ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,cchar *) ;
static int	subinfo_xfile(SUBINFO *,cchar *) ;
static int	subinfo_record(SUBINFO *,cchar *,int) ;
static int	subinfo_finish(SUBINFO *) ;

static int	subinfo_tryfull(SUBINFO *,char *,cchar *,int) ;
static int	subinfo_tryroot(SUBINFO *,char *,cchar *,int) ;
static int	subinfo_tryother(SUBINFO *,char *,cchar *,int) ;
static int	subinfo_tryothercheck(SUBINFO *,cchar *,int,
			char *,cchar *,int) ;

static int	mkdfname(char *,cchar *,int,cchar *,int) ;
static int	isOverNoEntAcc(int) ;


/* local variables */

static int 	(*tries[])(SUBINFO *,char *,cchar *,int) = {
	subinfo_tryfull,
	subinfo_tryroot,
	subinfo_tryother,
	NULL
} ;

static cchar	*prbins[] = {
	"bin",
	"sbin",
	NULL
} ;

static const int	rsentacc[] = {
	SR_NOENT,
	SR_ACCESS,
	SR_OVERFLOW,
	SR_NAMETOOLONG,
	SR_RANGE,
	0
} ;


/* exported subroutines */


int prgetprogpath(cchar *pr,char *rbuf,cchar *np,int nl)
{
	SUBINFO		si, *sip = &si ;
	int		rs ;
	int		rs1 ;
	int		rl = 0 ;

#if	CF_DEBUGN
	nprintf(NDF,"prgetprogpath: ent\n") ;
	nprintf(NDF,"prgetprogpath: pr=%s\n",pr) ;
	nprintf(NDF,"prgetprogpath: n=%t\n",np,nl) ;
#endif

	if (pr == NULL) return SR_FAULT ;
	if (rbuf == NULL) return SR_FAULT ;
	if (np == NULL) return SR_FAULT ;

	if (np[0] == '\0') return SR_INVALID ;

	if (nl < 0) nl = strlen(np) ;

	while ((nl > 0) && (np[nl-1] == '/')) {
	    sip->f_changed = TRUE ;
	    nl -= 1 ;
	}

	rbuf[0] = '\0' ;
	if ((rs = subinfo_start(sip,pr)) >= 0) {
	    int	i ;
	    for (i = 0 ; tries[i] != NULL ; i += 1) {
		rs = (*tries[i])(sip,rbuf,np,nl) ;
		rl = rs ;
		if ((rs != 0) || sip->f_done) break ;
	    } /* end for */
	    if (rs >= 0) {
		if (rl == 0) {
		    rs = SR_NOENT ;
 		} else if (rl > 0) {
		    if (! sip->f_changed) rl = 0 ;
		}
	    }
	    rs1 = subinfo_finish(sip) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (subinfo) */

#if	CF_DEBUGS
	debugprintf("prgetprogpath: ret rs=%d rl=%u\n",rs,rl) ;
	debugprintf("prgetprogpath: rbuf=%s\n",rbuf) ;
#endif
#if	CF_DEBUGN
	nprintf(NDF,"prgetprogpath: ret rs=%d rl=%u\n",rs,rl) ;
	nprintf(NDF,"prgetprogpath: rbuf=%s\n",rbuf) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (prgetprogpath) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,cchar *pr)
{
	int		rs ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->pr = pr ;

	rs = ids_load(&sip->id) ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->f_dirs) {
	    sip->f_dirs = FALSE ;
	    rs1 = vecstr_finish(&sip->dirs) ;
	    if (rs >= 0) rs = rs1 ;
	}

	rs1 = ids_release(&sip->id) ;
	if (rs >= 0) rs = rs1 ;

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_tryfull(SUBINFO *sip,char *rbuf,cchar *np,int nl)
{
	int		rs = SR_OK ;
	int		rl = 0 ;
	if (strnchr(np,nl,'/') != NULL) {
	    if ((rs = mkpath1w(rbuf,np,nl)) >= 0) {
		rl = rs ;
		rs = subinfo_xfile(sip,rbuf) ;
	        if (isNotPresent(rs)) {
		    rs = SR_OK ;
		    rl = 0 ;
		    sip->f_done = TRUE ;
		}
	    }
	} /* end if (full-path) */
	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (subinbfo_tryfull) */


static int subinfo_tryroot(SUBINFO *sip,char *rbuf,cchar *np,int nl)
{
	int		rs = SR_OK ;
	int		i ;
	int		rl = 0 ;
	cchar		*pr = sip->pr ;

#if	CF_DEBUGS
	debugprintf("prgetprogpath/subinfo_tryroot: n=%t\n",np,nl) ;
#endif

	for (i = 0 ; prbins[i] != NULL ; i += 1) {
	    rl = 0 ;
	    if ((rs = mkpath2(rbuf,pr,prbins[i])) >= 0) {
	        const int	plen = rs ;
		if ((rs = pathaddw(rbuf,plen,np,nl)) >= 0) {
		    rl = rs ;
	            if ((rs = subinfo_xfile(sip,rbuf)) >= 0) {
			break ;
		    } else if (isOverNoEntAcc(rs)) {
	                rs = subinfo_record(sip,rbuf,rl) ;
			rl = 0 ;
		    }
		}
	    } else if (isOverNoEntAcc(rs)) {
	        rs = SR_OK ;
	    }
	    if (rs < 0) break ;
	} /* end for */

	if ((rs >= 0) && (rl > 0)) {
	    sip->f_changed = TRUE ;
	}

#if	CF_DEBUGS
	debugprintf("prgetprogpath/subinfo_tryroot: rs=%d ol=%u\n",rs,rl) ;
	debugprintf("prgetprogpath/subinfo_tryroot: rbuf=%s\n",rbuf) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (subinfo_tryroot) */


static int subinfo_tryother(SUBINFO *sip,char *rbuf,cchar *np,int nl)
{
	int		rs = SR_OK ;
	int		rl = 0 ;
	cchar		*sp ;

	if ((sp = getenv(VARPATH)) != NULL) {
	    cchar	*tp ;

	    while (((tp = strpbrk(sp,":;")) != NULL) && (rl == 0)) {
	        rs = subinfo_tryothercheck(sip,sp,(tp - sp),rbuf,np,nl) ;
	        rl = rs ;
	        sp = (tp + 1) ;
		if (rs < 0) break ;
	    } /* end while */

	    if ((rs >= 0) && (rl == 0)) {
	        if (sp[0] != '\0') {
	            rs = subinfo_tryothercheck(sip,sp,-1,rbuf,np,nl) ;
	            rl = rs ;
	        }
	    }

	    if ((rs >= 0) && (rl > 0)) {
	        sip->f_changed = TRUE ;
	    }

	} /* end if (getenv-path) */

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (subinfo_other) */


static int subinfo_tryothercheck(SUBINFO *sip,cchar *dp,int dl,
		char *rbuf,cchar *np,int nl)
{
	int		rs = SR_NOENT ;
	int		rl = 0 ;

#if	CF_DEBUGS
	debugprintf("prgetprogpath/subinfo_tryothercheck: dl=%d d=%t\n",
	    dl,dp,strnnlen(dp,dl,40)) ;
#endif

	if (sip->f_dirs) {
	    rs = vecstr_findn(&sip->dirs,dp,dl) ;
	}

	if (rs == SR_NOENT) {
	    if ((rs = mkdfname(rbuf,dp,dl,np,nl)) >= 0) {
	        rl = rs ;
	        rs = subinfo_xfile(sip,rbuf) ;
	        if ((rs == SR_NOENT) || (rs == SR_ACCESS)) {
	            rs = subinfo_record(sip,rbuf,rl) ;
		    rl = 0 ;
		}
	    } /* end if */
	} /* end if (no-entry) */

#if	CF_DEBUGS
	debugprintf("prgetprogpath/subinfo_tryothercheck: ret rs=%d rl=%u\n",
	    rs,rl) ;
#endif

	return (rs >= 0) ? rl : rs ;
}
/* end subroutine (subinfo_tryothercheck) */


static int subinfo_xfile(SUBINFO *sip,cchar name[])
{
	struct ustat	sb ;
	int		rs ;

	if ((rs = u_stat(name,&sb)) >= 0) {
	    rs = SR_NOENT ;
	    if (S_ISREG(sb.st_mode)) {
	        rs = sperm(&sip->id,&sb,X_OK) ;
	    }
	}

	return rs ;
}
/* end subroutine (subinfo_xfile) */


static int subinfo_record(SUBINFO *sip,cchar dp[],int dl)
{
	int		rs = SR_OK ;

	if (! sip->f_dirs) {
	    rs = vecstr_start(&sip->dirs,10,0) ;
	    sip->f_dirs = (rs >= 0) ;
	}

	if (rs >= 0) {
	    rs = vecstr_add(&sip->dirs,dp,dl) ;
	}

	return rs ;
}
/* end subroutine (subinfo_record) */


static int mkdfname(char rbuf[],cchar dp[],int dl,cchar *np,int nl)
{
	const int	rlen = MAXPATHLEN ;
	int		rs = SR_OK ;
	int		i = 0 ;
	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,dp,dl) ;
	    i += rs ;
	}
	if ((rs >= 0) && (i > 0) && (rbuf[i - 1] != '/')) {
	    rs = storebuf_char(rbuf,rlen,i,'/') ;
	    i += rs ;
	}
	if (rs >= 0) {
	    rs = storebuf_strw(rbuf,rlen,i,np,nl) ;
	    i += rs ;
	}
	return (rs >= 0) ? i : rs ;
}
/* end subroutine (mkdfname) */


static int isOverNoEntAcc(int rs)
{
	return isOneOf(rsentacc,rs) ;
}
/* end subroutine (isOverNoEntAcc) */



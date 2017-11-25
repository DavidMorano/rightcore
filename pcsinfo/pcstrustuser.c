/* pcstrustuser */

/* is a user a PCS-trusted user */


#define	CF_DEBUGS	0		/* non-switchable */
#define	CF_PCSGROUP	1		/* include pcs-group check */
#define	CF_UGETPW	1		/* use |ugetpw(3uc)| */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This code module was completely rewritten to replace any original
	garbage that was here before.

	= 2013-06-04, David A­D­ Morano
	"Turn down the heat!"  That is what I used to say when one of my
	circuits that I was designing started to get a little too heavy in the
	power-consumption department!  This subroutine here was a little bit
	that way; namely, overly heavy!  I have turned down the heat a bit by
	rewriting this subroutine substantially and making it into a more
	simple process of finding whether a username can be trusted or not.  It
	isn't like this subroutine is used everywhere, so let's not overdue
	this whole process.

*/

/* Copyright © 1998,2013 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine checks to see if a user is a PCS-trusted user.

	Notes:

	There is not strict order needed to find the "correct" thing here.  We
	just search to see if the given username is supposed to be trusted or
	not (a binary decision).  So order of searching is flexible.  Since we
	can search in whatever order we want, what we are going to try to do
	here is to search in the order of increasing complexity (increasing
	time) of search.

	Is going to the system name-server cache faster than going to a file in
	the file-system?  In theory the name-server cache could respond with an
	answer *without* ever going to any file-systems, so we are going to
	consider the name-server faster than a file-system file lookup.  You
	mileage may vary.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<vecstr.h>
#include	<getax.h>
#include	<ugetpw.h>
#include	<getxusername.h>
#include	<localmisc.h>


/* local defines */

#if	CF_UGETPW
#define	GETPW_NAME	ugetpw_name
#else
#define	GETPW_NAME	getpw_name
#endif /* CF_UGETPW */

#define	TRUSTFNAME	"trusted"

#define	SUBINFO		struct subinfo


/* external subroutines */

extern int	mkpath2(char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	vecstr_envset(vecstr *,const char *,const char *,int) ;
extern int	vecstr_loadfile(vecstr *,int,const char *) ;
extern int	permsched(const char **,vecstr *,char *,int,const char *,int) ;
extern int	isNotPresent(int) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* local structures */

struct subinfo_flags {
	uint		id_pr:1 ;
	uint		id_un:1 ;
} ;

struct subinfo {
	const char	*pr ;		/* passed argument */
	const char	*un ;		/* passed argument */
	struct subinfo_flags	f ;
	uid_t		uid_pr, uid_un ;
	gid_t		gid_pr, gid_un ;
	char		unbuf[USERNAMELEN+1] ;
} ;


/* local typedefs */


/* forward references */

static int	subinfo_start(SUBINFO *,const char *,const char *) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_idpr(SUBINFO *) ;
static int	subinfo_idun(SUBINFO *) ;
static int	subinfo_listdb(SUBINFO *) ;
static int	subinfo_filedb(SUBINFO *) ;

#if	CF_PCSGROUP
static int	subinfo_prgroup(SUBINFO *) ;
#endif

/* local variables */

static const char	*trustedusers[] = {
	"root",
	"uucp",
	"nuucp",
	"adm",
	"admin",
	"daemon",
	"listen",
	"pcs",
	"post",
	"genserv",
	"local",
	"ncmp",
	"dam",
	"morano",
	NULL
} ;

static const char	*sched[] = {
	"%r/etc/pcs.%f",
	"%r/etc/%f",
	NULL
} ;

static int	(*tries[])(SUBINFO *) = {
	subinfo_listdb,
#if	CF_PCSGROUP
	subinfo_prgroup,
#endif
	subinfo_filedb,
	NULL
} ;


/* exported subroutines */


int pcstrustuser(cchar pr[],cchar un[])
{
	SUBINFO		si ;
	int		rs ;

	if (pr == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;
	if (un[0] == '\0') return SR_INVALID ;

	if ((rs = subinfo_start(&si,pr,un)) >= 0) {
	    int	i ;

	    for (i = 0 ; (rs >= 0) && (tries[i]) != NULL; i += 1) {
	        rs = (*tries[i])(&si) ;
	        if (rs > 0) break ;
	    }

	    subinfo_finish(&si) ;
	} /* end if (subinfo) */

	return rs ;
}
/* end subroutine (pcstrustuser) */


/* local subroutines */


static int subinfo_start(SUBINFO *sip,const char *pr,const char *un)
{
	int		rs = SR_OK ;

	memset(sip,0,sizeof(SUBINFO)) ;
	sip->pr = pr ;
	sip->un = un ;
	sip->uid_pr = -1 ;
	sip->gid_pr = -1 ;
	sip->uid_un = -1 ;
	sip->gid_un = -1 ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_finish(SUBINFO *sip)
{

	if (sip == NULL) return SR_FAULT ;

	return SR_OK ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_listdb(SUBINFO *sip)
{
	int		rs = SR_OK ;
	const char	*un = sip->un ;

	if (un[0] == '-') {
	    rs = subinfo_idun(sip) ;
	    un = sip->un ;
	}

	if (rs >= 0) {
	    int	i = matstr(trustedusers,un,-1) ;
	    if (i >= 0) rs = 1 ;
	}

	return rs ;
}
/* end subroutine (subinfo_listdb) */


static int subinfo_filedb(SUBINFO *sip)
{
	VECSTR		svs ;
	int		rs ;
	int		rs1 ;
	int		f = FALSE ;
	const char	*tfname = TRUSTFNAME ;
	const char	*pr = sip->pr ;
	const char	*un = sip->un ;

	if ((rs = vecstr_start(&svs,6,0)) >= 0) {
	    if ((rs = vecstr_envset(&svs,"r",pr,-1)) >= 0) {
	        const int	tlen = MAXPATHLEN ;
	        char		tbuf[MAXPATHLEN + 1] ;
	        if ((rs1 = permsched(sched,&svs,tbuf,tlen,tfname,R_OK)) >= 0) {

	            if (un[0] == '-') {
	                rs = subinfo_idun(sip) ;
	                un = sip->un ;
	            }

	            if (rs >= 0) {
			VECSTR		tusers ;
	                if ((rs = vecstr_start(&tusers,10,0)) >= 0) {

	                    if ((rs = vecstr_loadfile(&tusers,0,tbuf)) >= 0) {
	                        rs1 = vecstr_find(&tusers,un) ;
	                        f = (rs1 >= 0) ;
	                    }

	                    vecstr_finish(&tusers) ;
	                } /* end if (vecstr) */
	            } /* end if (ok) */

	        } /* end if (successful permsched) */
	    } /* end if (vecstr_envset) */
	    rs1 = vecstr_finish(&svs) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (svs) */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (subinfo_filedb) */


static int subinfo_idpr(SUBINFO *sip)
{
	int		rs = SR_OK ;

	if (! sip->f.id_pr) {
	    struct ustat	sb ;
	    const char		*pr = sip->pr ;
	    sip->f.id_pr = TRUE ;
	    if (u_stat(pr,&sb) >= 0) {
	        sip->uid_pr = sb.st_uid ;
	        sip->gid_pr = sb.st_gid ;
	    }
	}

	return rs ;
}
/* end subroutine (subinfo_idpr) */


static int subinfo_idun(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		rs1 ;
	int		len = 0 ;

	if (! sip->f.id_un) {
	    struct passwd	pw ;
	    const int		pwlen = getbufsize(getbufsize_pw) ;
	    const char		*un = sip->un ;
	    char		*pwbuf ;
	    sip->f.id_un = TRUE ;
	    if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	        if (un[0] == '-') {
	            if ((rs = getpwusername(&pw,pwbuf,pwlen,-1)) >= 0) {
			const int	ulen = USERNAMELEN ;
			cchar		*un = pw.pw_name ;
	                len = (strwcpy(sip->unbuf,un,ulen) - sip->unbuf) ;
	                sip->un = sip->unbuf ;
		    }
	        } else {
	            rs = GETPW_NAME(&pw,pwbuf,pwlen,un) ;
	        }
	        if (rs >= 0) {
	            sip->uid_un = pw.pw_uid ;
	            sip->gid_un = pw.pw_gid ;
	        } else if (isNotPresent(rs)) {
		    rs = SR_OK ;
		}
		rs1 = uc_free(pwbuf) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (m-a) */
	} else if (sip->un != NULL) {
	    len = strlen(sip->un) ;
	} /* end if (needed) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (subinfo_idun) */


#if	CF_PCSGROUP

static int subinfo_prgroup(SUBINFO *sip)
{
	int		rs = SR_OK ;
	int		f = FALSE ;

	if (! sip->f.id_pr) rs = subinfo_idpr(sip) ;

	if (! sip->f.id_un) rs = subinfo_idun(sip) ;

	if ((rs >= 0) && (! f)) {
	    f = (sip->gid_un == sip->gid_pr) ;
	}

/* check if username is in the PCS group */

	if ((rs >= 0) && (! f)) {
	    struct group	gr ;
	    const int		grlen = getbufsize(getbufsize_gr) ;
	    int			rs1 ;
	    int			i ;
	    char		*grbuf ;
	    if ((rs = uc_malloc((grlen+1),&grbuf)) >= 0) {
	        if ((rs1 = uc_getgrgid(sip->gid_pr,&gr,grbuf,grlen)) >= 0) {
	            for (i = 0 ; gr.gr_mem[i] != NULL ; i += 1) {
	                f = (strcmp(gr.gr_mem[i],sip->un) == 0) ;
	                if (f) break ;
	            }
	        } /* end if (got a group for PR) */
		uc_free(grbuf) ;
	    } /* end if (m-a) */
	} /* end if */

	return (rs >= 0) ? f : rs ;
}
/* end subroutine (pcstrustuser) */

#endif /* CF_PCSGROUP */



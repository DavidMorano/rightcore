/* checkpass */

/* check a password for validity */


#define	CF_DEBUG	0		/* run-time debugging */
#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1996-03-01, David A­D­ Morano
        The program was written from scratch to do what the previous program by
        the same name did.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will take as input a username and a password and it will
        return a boolean about whether or not the password is valid on the
        current system.


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<sys/wait.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pwd.h>
#include	<shadow.h>
#include	<grp.h>

#include	<vsystem.h>
#include	<getbufsize.h>
#include	<bfile.h>
#include	<pwfile.h>
#include	<vecstr.h>
#include	<des.h>
#include	<getax.h>
#include	<spawnproc.h>
#include	<filebuf.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	BUFLEN
#define	BUFLEN		MAXPATHLEN
#endif

#ifndef	RDBUFLEN
#define	RDBUFLEN	80
#endif

#ifndef	PROG_NISMATCH
#define	PROG_NISMATCH	"/usr/bin/nismatch"
#endif

#define	TO_READ		5

#define	PASSDATA	struct passdata


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sfshrink(const char *,int,char **) ;
extern int	matstr(const char **,const char *,int) ;
extern int	havenis() ;

extern int	passwdok(const char *,const char *) ;


/* external variables */


/* local structures */

struct passdata {
	const char	*username ;
	char		*passbuf ;
} ;


/* forward references */

static int	try_start(PASSDATA *,char *,const char *) ;
static int	try_pwd(PASSDATA *) ;
static int	try_spw(PASSDATA *) ;
static int	try_nisplus(PASSDATA *) ;
static int	try_finish(PASSDATA *) ;

static int	readpass(int,char *,int) ;


/* local variables */

static cchar	*badpasswds[] = {
	"x",
	"*NP*",
	"NP",
	NULL
} ;

static int	(*tries[])(PASSDATA *) = {
	try_pwd,
	try_spw,
	try_nisplus,
	NULL
} ;


/* exported subroutines */


int checkpass(PROGINFO *pip,cchar *username,cchar *password,int f_useshadow)
{
	PASSDATA	pd, *pdp = &pd ;
	int		rs ;
	int		rs1 = 0 ;
	int		i ;
	int		passlen = 0 ;
	int		f_valid = FALSE ;
	char		passbuf[PASSWDLEN + 1] ;

	if (username == NULL) return SR_FAULT ;
	if (password == NULL) return SR_FAULT ;

	if (username[0] == '\0') return SR_INVALID ;

	if (pip->debuglevel > 0) {
	    bprintf(pip->efp,"%s: f_useshadow=%u\n",
	        pip->progname,f_useshadow) ;
	}

	if ((rs = try_start(pdp,passbuf,username)) >= 0) {

	for (i = 0 ; tries[i] != NULL ; i += 1) {

	    rs1 = (*tries[i])(pdp) ;
	    passlen = rs1 ;

	    if ((rs1 >= 0) || (rs1 == SR_NOMEM))
	        break ;

	} /* end for */

	if ((rs >= 0) && (rs1 == SR_NOMEM))
	    rs = SR_NOMEM ;

	try_finish(pdp) ;
	} /* end if */

	if ((pip->debuglevel > 0) && (rs >= 0))
	    bprintf(pip->efp,"%s: system password=>%s<\n",
	        pip->progname,passbuf) ;

/* check if the password is OK */

	if ((rs >= 0) && (passlen > 0)) {
	    rs1 = passwdok(password,passbuf) ;
	    f_valid = (rs1 > 0) ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(3))
	    debugprintf("checkpass: ret rs=%d f_valid=%u\n",
	        rs,f_valid) ;
#endif

	return (rs >= 0) ? f_valid : rs ;
}
/* end subroutine (checkpass) */


/* local subroutines */


static int try_start(PASSDATA *pdp,char *passbuf,cchar *username)
{

	if (pdp == NULL) return SR_FAULT ;
	if (username == NULL) return SR_FAULT ;

	passbuf[0] = '\0' ;
	memset(pdp,0,sizeof(PASSDATA)) ;

	pdp->username = username ;
	pdp->passbuf = passbuf ;
	return SR_OK ;
}
/* end subroutine (try_start) */


static int try_finish(PASSDATA *pdp)
{

	if (pdp == NULL) return SR_FAULT ;

	memset(pdp,0,sizeof(PASSDATA)) ;

	return SR_OK ;
}
/* end subroutine (try_finish) */


static int try_pwd(PASSDATA *pdp)
{
	struct passwd	pw ;
	const int	pwlen = getbufsize(getbufsize_pw) ;
	int		rs ;
	char		*pwbuf ;

	pdp->passbuf[0] = '\0' ;
	if ((rs = uc_malloc((pwlen+1),&pwbuf)) >= 0) {
	    cchar	*un = pdp->username ;
	    if ((rs = getpw_name(&pw,pwbuf,pwlen,un)) >= 0) {
	        rs = SR_NOEXIST ;
	        if (matstr(badpasswds,pw.pw_passwd,-1) < 0) {
	            rs = sncpy1(pdp->passbuf,PASSWDLEN,pw.pw_passwd) ;
		}
	    }
	    uc_free(pwbuf) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (try_pwd) */


static int try_spw(PASSDATA *pdp)
{
	struct spwd	sp ;
	const int	splen = getbufsize(getbufsize_sp) ;
	int		rs ;
	char		*spbuf ;

	pdp->passbuf[0] = '\0' ;
	if ((rs = uc_malloc((splen+1),&spbuf)) >= 0) {
	    cchar	*un = pdp->username ;
	    if ((rs = getsp_name(&sp,spbuf,splen,un)) >= 0) {
	        rs = SR_NOEXIST ;
	        if (matstr(badpasswds,sp.sp_pwdp,-1) < 0) {
	            rs = sncpy1(pdp->passbuf,PASSWDLEN,sp.sp_pwdp) ;
		}
	    }
	    uc_free(spbuf) ;
	} /* end if (memory-allocation) */

	return rs ;
}
/* end subroutine (try_spw) */


static int try_nisplus(PASSDATA *pdp)
{
	SPAWNPROC	ps ;
	vecstr		args ;
	pid_t		cpid ;
	int		rs ;
	int		len ;
	int		cstat ;
	const char	*progfname = PROG_NISMATCH ;
	const char	**av ;

#if	CF_DEBUGS
	debugprintf("checkpass/try_nisplus: ent\n") ;
#endif

	rs = havenis() ;
	if (rs < 0)
	    goto ret0 ;

	pdp->passbuf[0] = '\0' ;
	rs = vecstr_start(&args,5,0) ;
	if (rs < 0)
	    goto ret0 ;

	rs = vecstr_add(&args,"nismatch",-1) ;

	if (rs >= 0)
	    rs = vecstr_add(&args,pdp->username,-1) ;

	if (rs >= 0)
	    rs = vecstr_add(&args,"passwd.org_dir",-1) ;

	if (rs < 0)
	    goto ret1 ;

/* try to execute NISMATCH */

	vecstr_getvec(&args,&av) ;

	memset(&ps,0,sizeof(SPAWNPROC)) ;
	ps.disp[0] = SPAWNPROC_DCLOSE ;
	ps.disp[1] = SPAWNPROC_DCREATE ;
	ps.disp[2] = SPAWNPROC_DCLOSE ;
	if ((rs = spawnproc(&ps,progfname,av,NULL)) >= 0) {
	    const int	fd = ps.fd[1] ;
	    int		cex ;
	    cpid = rs ;

	    rs = readpass(fd,pdp->passbuf,PASSWDLEN) ;
	    len = rs ;

	    u_close(fd) ;

	    u_waitpid(cpid,&cstat,WUNTRACED) ;

	    if (rs >= 0) {
	        if (WIFEXITED(cstat)) {
		    cex = WEXITSTATUS(cstat) ;
		    if (cex > 0)
		        rs = SR_NOEXIST ;
		} else
		    rs = SR_NOEXIST ;
	    } 

	} /* end if */

	if ((rs >= 0) && (len > 0) &&
	    (matstr(badpasswds,pdp->passbuf,len) >= 0))
	    rs = SR_NOEXIST ;

ret1:
	vecstr_finish(&args) ;

ret0:

#if	CF_DEBUGS
	debugprintf("checkpass/try_nisplus: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (try_nisplus) */


static int readpass(int fd,char *outbuf,int outlen)
{
	FILEBUF		b ;
	int		rs ;
	int		len ;
	int		cl = -1 ;
	const char	*tp ;
	const char	*cp = NULL ;
	char		rdbuf[RDBUFLEN + 1] ;

	if (outbuf == NULL) return SR_FAULT ;

	if (fd < 0) {
	    rs = SR_BADF ;
	    goto ret0 ;
	}

	rs = filebuf_start(&b,fd,0,-1,0) ;
	if (rs < 0)
	    goto ret0 ;

	rs = filebuf_readline(&b,rdbuf,RDBUFLEN,TO_READ) ;
	len = rs ;
	if (rs >= 0) {

	        if (rdbuf[len - 1] == '\n')
	            len -= 1 ;

	        rdbuf[len] = '\0' ;
	        cp = NULL ;
	        cl = -1 ;
	        if ((tp = strchr(rdbuf,':')) != NULL) {
		    cp = (tp + 1) ;
		    cl = -1 ;
	            if ((tp = strchr(cp,':')) != NULL) {
			cl = (tp - cp) ;
		    }
		}

	} /* end if (reading line) */

	filebuf_finish(&b) ;

	if (rs >= 0) {
	    rs = SR_NOEXIST ;
	    if ((cp != NULL) && (cl >= 0))
		rs = snwcpy(outbuf,outlen,cp,cl) ;
	}

ret0:
	return rs ;
}
/* end subroutine (readpass) */




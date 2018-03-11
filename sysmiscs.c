/* sysmiscs */

/* return SYSMISC information from the system */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will find and return the number of CPUs in
	the present system.

	Synopsis:

	int sysmiscs(pr)
	const char	pr[] ;

	Arguments:

	pr		program root

	Returns:

	>0		OK and this is the number of CPUs in the system
	==0		could not determine the number of CPUs
	<0		error code


*******************************************************************************/


#include	<envstandards.h>	/* must be before others */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<limits.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<time.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<endianstr.h>
#include	<vecobj.h>
#include	<msfile.h>
#include	<spawnproc.h>
#include	<localmisc.h>


/* local defines */

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	120
#endif

#define	TO_OLDFILE	(5 * 60)
#define	TO_MSOLD	10

#define	MSVARDNAME	"var"
#define	MSFNAME		"ms"
#define	MSBINDNAME	"bin"

#define	DEBFNAME	"sysvar.deb"


/* external subroutines */

extern uint	hashelf(const char *,int) ;
extern uint	hashagain(uint,int,int) ;
extern uint	nextpowtwo(uint) ;

extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	sncpy4(char *,int,const char *,const char *,const char *,
			const char *) ;
extern int	sncpy5(char *,int,const char *,const char *,const char *,
			const char *,const char *) ;
extern int	sncpy6(char *,int,const char *,const char *,const char *,
			const char *,const char *,const char *) ;
extern int	snwcpy(char *,int,const char *,int) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	mkpath1w(char *,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	sfword(const char *,int,const char **) ;
extern int	strnnlen(const char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	getnodename(char *,int) ;
extern int	getpwd(char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	opentmpfile(const char *,int,int,char *) ;
extern int	hasuc(const char *,int) ;
extern int	vstrkeycmp(const char *,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;


/* external variables */


/* exported variables */

MODLOAD_MID	sysmiscs = {
	"sysmiscs",
	sizeof(SYSMISCS)
} ;


/* local structures */


/* forward references */

static int	sysmiscs_msopen(SYSMISCS *) ;
static int	sysmiscs_msget(SYSMISCS *,time_t) ;
static int	sysmiscs_msgetone(SYSMISCS *,time_t,SYSMISCS_DATA *) ;
static int	sysmiscs_nodename(SYSMISCS *) ;


/* exported variables */

SYSMISCS_OBJ	sysmiscs = {
	"sysmiscs",
	sizeof(SYSMISCS)
} ;


/* local variables */

static const char	*prbins[] = {
	"bin",
	"sbin",
	NULL
} ;


/* exported subroutines */


int sysmiscs_open(op,pr)
SYSMISCS	*op ;
const char	pr[] ;
{
	SYSMISCS	si ;

	int	rs = SR_OK ;
	int	n = 0 ;
	const char	*cp ;

	if (op == NULL)
	    return SR_FAULT ;

	if (pr == NULL)
	    return SR_FAULT ;

	memset(op,0,sizeof(SYSMISCS)) ;

	if ((rs = uc_mallocstrw(pr,-1,&cp)) >= 0) {
	    op->pr = cp ;
	    op->magic = SYSMISCS_MAGIC ;
	}

#if	CF_DEBUGS
	debugprintf("sysmiscs_open: ret rs=%d\n",rs) ;
#endif

	return rs ;

/* bad stuff */
bad0:
	goto ret0 ;
}
/* end subroutine (sysmiscs_open) */


int sysmiscs_get(op,daytime,dp)
SYSMISCS	*op ;
time_t		daytime ;
SYSMISCS_DATA	*dp ;
{
	int	rs = SR_OK ;
	int	n = 0 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SYSMISCS_MAGIC)
	    return SR_NOTOPEN ;

	if (daytime == 0) daytime = time(NULL) ;

	rs = sysmiscs_msget(op,daytime,dp) ;
	n = rs ;

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (sysmiscs_get) */


int sysmiscs_close(op)
SYSMISCS	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SYSMISCS_MAGIC)
	    return SR_NOTOPEN ;


	if (op->f.msopen) {
	    op->f.msinit = FALSE ;
	    op->f.msopen = FALSE ;
	    rs1 = msinfo_close(&op->ms) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->nodename != NULL) {
	    uc_free(op->nodename) ;
	    op->nodename = NULL ;
	}

	if (op->pr != NULL) {
	    uc_free(op->pr) ;
	    op->pr = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (sysmiscs_close) */


/* private subroutines */


static int sysmiscs_msget(op,daytime,dp)
SYSMISCS	*op ;
time_t		daytime ;
SYSMISCS_DATA	*dp ;
{
	int	rs = SR_OK ;
	int	nl ;
	int	n = 0 ;

	const char	*nn ;


	if (! op->f.msinit)
	    rs = sysmiscs_msopen(op) ;

	if (rs < 0)
	    goto ret0 ;

	if (op->f.msopen) {

	    rs = sysmiscs_msgetone(op,daytime,dp) ;

	} /* end if (MS opened) */

	if (rs < 0)
	    goto ret0 ;

	if ((! op->f.msopen) || op->f.msold) {
	    rs = sysmiscs_msdaemon(op) ;
	}

ret0:
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (sysmiscs_msget) */


static int sysmiscs_msopen(op)
SYSMISCS	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	oflags = O_RDONLY ;
	int	operms = 0666 ;

	char	fname[MAXPATHLEN + 1] ;


	if (op->f.msinit)
	    goto ret0 ;

	op->f.msinit = TRUE ;
	rs = mkpath3(fname,op->pr,MSVARDNAME,MSFNAME) ;
	if (rs >= 0) {
	    rs1 = msfile_open(&op->msfile,fname,oflags,operm)
	    op->f.msopen = (rs1 >= 0) ;
	}

ret0:
	return rs ;
}
/* end subroutine (sysmiscs_msopen) */


static int sysmiscs_msgetone(op,daytime,dp)
SYSMISCS	*op ;
time_t		daytime ;
SYSMISCS_DATA	*dp ;
{
	MSFILE_ENT	e ;

	int	rs ;
	int	rs1 ;


	rs = sysmiscs_nodename(op) ;
	if (rs < 0)
	    goto ret0 ;

	nn = op->nodename ;
	nl = op->nodenamelen ;
	rs1 = msfile_match(&op->msfile,0L,nn,nl,&e) ;
	
	if (rs1 >= 0) {

	    if (daytime == 0) daytime = time(NULL) ;

	    op->f.msold = ((daytime - e.utime) >= TO_MSOLD) ;

	} else if (rs1 == SR_NOTFOUND) {
	    op->f.msold = TRUE ;
	} else
	    rs = rs1 ;

ret0:
	return rs ;
}
/* end subroutine (sysmiscs_msgetone) */


static int sysmiscs_nodename(op)
SYSMISCS	*op ;
{
	int	rs = SR_OK ;
	int	nlen ;

	char	nbuf[NODENAMELEN + 1] ;
	char	*p ;


	if (op->nodename != NULL)
	    goto ret0 ;

	rs = getnodename(nbuf,NODENAMELEN) ;
	nlen = rs ;
	if (rs >= 0) {
	    rs = uc_mallocstrw(nbuf,nlen,&p) ;
	    if (rs >= 0) {
		op->nodename = p ;
		op->nodenamelen = nlen ;
	    }
	}

ret0:
	return rs ;
}
/* end subroutine (sysmiscs_nodename) */


static int sysmiscs_msdaemon(op)
SYSMISCS	*op ;
{
	SPAWNPROC	ps ;

	pid_t	cpid ;

	int	rs = SR_OK ;
	int	i ;
	int	cstat ;
	int	cex ;

	const char	*pn = SYSMISCS_PROG ;
	const char	*av[3] ;
	const char	**ev ;

	char	progfname[MAXPATHLEN + 1] ;


	rs = mkpath3(progfname,op->pr,MSBINDNAME,pn) ;
	if (rs < 0)
	    goto ret0 ;

	av[0] = pn ;
	av[1] = "-o" ;
	av[2] = "quick" ;
	av[3] = "-d" ;

	vecstr_getvec(&envs,(const char ***) &ev) ;

	memset(&ps,0,sizeof(SPAWNPROC)) ;

	for (i = 0 ; i < 3 ; i += 1) {
	    ps.disp[i] = (i != 2) ? SPAWNPROC_DCLOSE : SPAWNPROC_DINHERIT ;
	}

	rs = spawnproc(&ps,progfname,av,ev) ;
	cpid = rs ;

ret2:
	vecstr_free(&envs) ;

ret1:
	if (rs < 0)
	    goto ret0 ;

	cstat = 0 ;
	rs = 0 ;
	while (rs == 0) {
	    rs = u_waitpid(cpid,&cstat,0) ;
	    if (rs == SR_INTR) rs = 0 ;
	} /* end while */

	if (rs >= 0) {

	    cex = 0 ;
	    if (WIFSIGNALED(cstat))
	        rs = SR_UNATCH ;	/* protocol not attached */

#if	CF_DEBUGS
	        debugprintf("bibleqs_mkbibleqsi: signaled? rs=%d\n",rs) ;
#endif

	    if ((rs >= 0) && WIFEXITED(cstat)) {

	        cex = WEXITSTATUS(cstat) ;

	        if (cex != 0)
	            rs = SR_LIBBAD ;

#if	CF_DEBUGS
	        debugprintf("bibleqs_mkbibleqsi: exited? cex=%d rs=%d\n",
			cex,rs) ;
#endif

	    }

	} /* end if (process finished) */

ret0:
	return rs ;
}
/* end subroutine (sysmiscs_msdaemon) */



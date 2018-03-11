/* sysmiscers */

/* return SYSMISC information from the system */


#define	CF_DEBUGS	0		/* compile-time debugging */


/* revision history:

	= 1998-03-01, David A­D­ Morano
	This subroutine was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

        This subroutine will find and return the number of CPUs in the present
        system.

	Synopsis:

	int sysmiscers(pr)
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
#include	<vecobj.h>
#include	<msfile.h>
#include	<vecstr.h>
#include	<ids.h>
#include	<spawnproc.h>
#include	<localmisc.h>

#include	"modload.h"
#include	"sysmiscers.h"


/* local defines */

#define	SYSMISCERS_MAGIC	0x66342125
#define	SYSMISCERS_MSPROG	"msu"
#define	SYSMISCERS_MSPR		"MSU_PROGRAMROOT"

#ifndef	ENDIANSTR
#ifdef	ENDIAN
#if	(ENDIAN == 0)
#define	ENDIANSTR	"0"
#else
#define	ENDIANSTR	"1"
#endif
#else
#define	ENDIANSTR	"1"
#endif
#endif

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
extern int	strnnlen(const char *,int,int) ;
extern int	matstr(const char **,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	getnodename(char *,int) ;
extern int	getpwd(char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	vecstr_envadd(vecstr *,const char *,const char *,int) ;
extern int	hasuc(const char *,int) ;
extern int	vstrkeycmp(const char *,const char *) ;
extern int	xfile(IDS *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* external variables */

extern char	**environ ;


/* exported variables */

MODLOAD_MID	sysmiscers = {
	"sysmiscers",
	sizeof(SYSMISCERS)
} ;


/* local structures */


/* forward references */

static int	sysmiscers_msopen(SYSMISCERS *) ;
static int	sysmiscers_msget(SYSMISCERS *,time_t,SYSMISCERS_DATA *) ;
static int	sysmiscers_msgetone(SYSMISCERS *,time_t,SYSMISCERS_DATA *) ;
static int	sysmiscers_nodename(SYSMISCERS *) ;
static int	sysmiscers_msdaemon(SYSMISCERS *) ;
static int	sysmiscers_msprogdaemon(SYSMISCERS *,char *,const char *) ;
static int	sysmiscers_envload(SYSMISCERS *,VECSTR *) ;

static int	istermrs(int) ;


/* local variables */

static const char	*prbins[] = {
	"bin",
	"sbin",
	NULL
} ;

static const char	*envbads[] = {
	"_",
	"_A0",
	"A__z",
	"SYSNAME",
	"RELEASE",
	"MACHINE",
	"NODE",
	"ARCHITECTURE",
	"HZ",
	"HOSTNAME",
	"TERMDEV",
	"TERM",
	"TERMCAP",
	"AUDIODEV",
	"DISPLAY",
	"RANDOM",
	"SECONDS",
	"LOGNAME",
	NULL
} ;

static const int	termrs[] = {
	SR_FAULT,
	SR_INVALID,
	SR_NOMEM,
	SR_NOANODE,
	SR_BADFMT,
	SR_NOSPC,
	SR_NOSR,
	SR_NOBUFS,
	SR_BADF,
	SR_OVERFLOW,
	SR_RANGE,
	0
} ;


/* exported subroutines */


int sysmiscers_open(op,pr)
SYSMISCERS	*op ;
const char	pr[] ;
{
	int	rs ;

	const char	*cp ;


	if (op == NULL) return SR_FAULT ;
	if (pr == NULL) return SR_FAULT ;

	memset(op,0,sizeof(SYSMISCERS)) ;

	if ((rs = uc_mallocstrw(pr,-1,&cp)) >= 0) {
	    op->pr = cp ;
	    op->magic = SYSMISCERS_MAGIC ;
	}

#if	CF_DEBUGS
	debugprintf("sysmiscers_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (sysmiscers_open) */


int sysmiscers_close(op)
SYSMISCERS	*op ;
{
	int	rs = SR_OK ;
	int	rs1 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SYSMISCERS_MAGIC)
	    return SR_NOTOPEN ;

	if (op->f.msopen) {
	    op->f.msinit = FALSE ;
	    op->f.msopen = FALSE ;
	    rs1 = msfile_close(&op->ms) ;
	    if (rs >= 0) rs = rs1 ;
	}

	if (op->nodename != NULL) {
	    rs1 = uc_free(op->nodename) ;
	    if (rs >= 0) rs = rs1 ;
	    op->nodename = NULL ;
	}

	if (op->pr != NULL) {
	    rs1 = uc_free(op->pr) ;
	    if (rs >= 0) rs = rs1 ;
	    op->pr = NULL ;
	}

	op->magic = 0 ;
	return rs ;
}
/* end subroutine (sysmiscers_close) */


int sysmiscers_get(op,daytime,dp)
SYSMISCERS	*op ;
time_t		daytime ;
SYSMISCERS_DATA	*dp ;
{
	int	rs = SR_OK ;
	int	n = 0 ;


	if (op == NULL)
	    return SR_FAULT ;

	if (op->magic != SYSMISCERS_MAGIC)
	    return SR_NOTOPEN ;

	if (daytime == 0) daytime = time(NULL) ;

	rs = sysmiscers_msget(op,daytime,dp) ;
	n = rs ;

	return (rs >= 0) ? n : rs ;
}
/* end subroutine (sysmiscers_get) */


/* private subroutines */


static int sysmiscers_msget(op,daytime,dp)
SYSMISCERS	*op ;
time_t		daytime ;
SYSMISCERS_DATA	*dp ;
{
	int	rs = SR_OK ;

	if (! op->f.msinit) rs = sysmiscers_msopen(op) ;

	if (rs >= 0) {
	    if (op->f.msopen) {
	        rs = sysmiscers_msgetone(op,daytime,dp) ;
	    } /* end if (MS opened) */
	    if ((rs >= 0) && ((! op->f.msopen) || op->f.msold)) {
	        rs = sysmiscers_msdaemon(op) ;
	    }
	}

	return rs ;
}
/* end subroutine (sysmiscers_msget) */


static int sysmiscers_msopen(op)
SYSMISCERS	*op ;
{
	const mode_t	operms = 0666 ;

	const int	oflags = O_RDONLY ;
	int	rs = SR_OK ;
	int	rs1 ;

	if (! op->f.msinit) {
	    char	fname[MAXPATHLEN + 1] ;
	    op->f.msinit = TRUE ;
	    if ((rs = mkpath3(fname,op->pr,MSVARDNAME,MSFNAME)) >= 0) {
	        rs1 = msfile_open(&op->ms,fname,oflags,operms) ;
	        op->f.msopen = (rs1 >= 0) ;
	    }
	}

	return rs ;
}
/* end subroutine (sysmiscers_msopen) */


static int sysmiscers_msgetone(op,daytime,dp)
SYSMISCERS	*op ;
time_t		daytime ;
SYSMISCERS_DATA	*dp ;
{
	int	rs ;
	int	rs1 ;

	if ((rs = sysmiscers_nodename(op)) >= 0) {
	    MSFILE_ENT	e ;
	    int		nl = op->nodenamelen ;
	    const char	*nn = op->nodename ;
	    if ((rs1 = msfile_match(&op->ms,0L,nn,nl,&e)) >= 0) {
	        if (daytime == 0) daytime = time(NULL) ;
	        op->f.msold = ((daytime - e.utime) >= TO_MSOLD) ;
	    } else if (rs1 == SR_NOTFOUND) {
	        op->f.msold = TRUE ;
	    } else
	        rs = rs1 ;
	}

	return rs ;
}
/* end subroutine (sysmiscers_msgetone) */


static int sysmiscers_nodename(op)
SYSMISCERS	*op ;
{
	int	rs = SR_OK ;
	int	nlen = 0 ;

	if (op->nodename == NULL) {
	    char	nbuf[NODENAMELEN + 1] ;
	    if ((rs = getnodename(nbuf,NODENAMELEN)) >= 0) {
	        const char	*cp ;
	        nlen = rs ;
	        if ((rs = uc_mallocstrw(nbuf,nlen,&cp)) >= 0) {
		    op->nodename = cp ;
		    op->nodenamelen = nlen ;
	        }
	    }
	} else
	    nlen = strlen(op->nodename) ;

	return (rs >= 0) ? nlen : rs ;
}
/* end subroutine (sysmiscers_nodename) */


static int sysmiscers_msdaemon(op)
SYSMISCERS	*op ;
{
	SPAWNPROC	ps ;

	VECSTR		envs ;

	pid_t	cpid ;

	int	rs = SR_OK ;
	int	i ;
	int	cstat ;
	int	cex ;

	const char	*pn = SYSMISCERS_MSPROG ;

	char	progfname[MAXPATHLEN + 1] ;


	rs = sysmiscers_msprogdaemon(op,progfname,pn) ;
	if (rs < 0) goto ret0 ;

	if ((rs = vecstr_start(&envs,150,0)) >= 0) {

	    if ((rs = sysmiscers_envload(op,&envs)) >= 0) {
	const char	*av[5] ;
	const char	**ev ;

	av[0] = pn ;
	av[1] = "-o" ;
	av[2] = "quick" ;
	av[3] = "-d" ;
	av[4] = NULL ;

	vecstr_getvec(&envs,&ev) ;

	memset(&ps,0,sizeof(SPAWNPROC)) ;

	for (i = 0 ; i < 3 ; i += 1) {
	    if (i != 2) {
	        ps.disp[i] = SPAWNPROC_DCLOSE ;
	    } else
	        ps.disp[i] = SPAWNPROC_DINHERIT ;
	}

	rs = spawnproc(&ps,progfname,av,ev) ;
	cpid = rs ;

	    } /* end if */

	    vecstr_finish(&envs) ;
	} /* end if (vecstr) */

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
/* end subroutine (sysmiscers_msdaemon) */


static int sysmiscers_msprogdaemon(op,progfname,pn)
SYSMISCERS	*op ;
char		progfname[] ;
const char	*pn ;
{
	IDS	id ;

	int	rs ;
	int	i ;
	int	len = 0 ;


	progfname[0] = '\0' ;
	if ((rs = ids_load(&id)) >= 0) {

	    for (i = 0 ; prbins[i] != NULL ; i += 1) {

	        rs = mkpath3(progfname,op->pr,prbins[i],pn) ;
	        len = rs ;
	        if (rs >= 0) 
		    rs = xfile(&id,progfname) ;

	        if ((rs >= 0) || istermrs(rs))
		    break ;

	    } /* end for */

	    ids_release(&id) ;
	} /* end if (ids) */

	return (rs >= 0) ? len : rs ;
}
/* end subroutine (sysmiscers_msprogdaemon) */


static int sysmiscers_envload(SYSMISCERS *op,VECSTR *elp)
{
	int	rs = SR_OK ;
	int	i ;
	int	c = 0 ;

	const char	*ep ;


	for (i = 0 ; (rs >= 0) && (environ[i] != NULL) ; i += 1) {
	    ep = environ[i] ;
	    if (matstr(envbads,ep,-1) < 0) {
		c += 1 ;
		rs = vecstr_add(elp,ep,-1) ;
	    }
	} /* end for */

	if (rs >= 0) {
	    const char	*varpr = SYSMISCERS_MSPROG ;
	    c += 1 ;
	    rs = vecstr_envadd(elp,varpr,op->pr,-1) ;
	}

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (sysmiscers_envload) */


static int istermrs(rs)
int	rs ;
{
	int	i ;
	int	f = FALSE ;


	for (i = 0 ; termrs[i] != 0 ; i += 1) {
	    f = (rs == termrs[i]) ;
	    if (f) break ;
	} /* end if */

	return f ;
}
/* end subroutine (istermrs) */



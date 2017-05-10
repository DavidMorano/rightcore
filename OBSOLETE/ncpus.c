/* ncpus */

/* return the number of CPUs in this system (this is a loadable module) */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_LATE		0		/* late open? */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	This subroutine was originally written.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine will find and return the number of CPUs in
	the present system.  This module is also loadable (as a module).

	Synopsis:

	int ncpus(pr)
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
#include	<ctype.h>

#include	<vsystem.h>
#include	<vecobj.h>
#include	<msfile.h>
#include	<localmisc.h>

#include	"modload.h"


/* local defines */

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

#define	SUBINFO		struct subinfo

#define	TO_OLDFILE	(5 * 60)
#define	TO_MSOLD	10

#define	MSDNAME		"var"
#define	MSFNAME		"ms"

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
extern int	mkpath1w(char *,const char *,int) ;
extern int	sfbasename(const char *,int,const char **) ;
extern int	sfdirname(const char *,int,const char **) ;
extern int	sfshrink(const char *,int,const char **) ;
extern int	strnnlen(const char *,int,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	cfdecui(const char *,int,uint *) ;
extern int	cfhexi(const char *,int,uint *) ;
extern int	getnodename(char *,int) ;
extern int	getpwd(char *,int) ;
extern int	perm(const char *,uid_t,gid_t,gid_t *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	hasuc(const char *,int) ;
extern int	vstrkeycmp(const char *,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;
extern char	*strwcpylc(char *,const char *,int) ;
extern char	*strnchr(const char *,int,int) ;
extern char	*strnpbrk(const char *,int,const char *) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
#endif


/* external variables */


/* exported variables */

MODLOAD_MID	ncpus = {
	"ncpus"
} ;


/* local structures */

struct subinfo_flags {
	uint		msinit:1 ;
	uint		msopen:1 ;
	uint		msold:1 ;
} ;

struct subinfo {
	const char	*pr ;
	const char	*nodename ;
	struct subinfo_flags	f ;
	MSFILE		msfile ;
	int		nodenamelen ;
} ;


/* forward references */

static int	subinfo_start(SUBINFO *,const char *) ;
static int	subinfo_msopen(SUBINFO *) ;
static int	subinfo_msget(SUBINFO *) ;
static int	subinfo_finish(SUBINFO *) ;
static int	subinfo_nodename(SUBINFO *) ;

#ifdef	COMMENT

static int	ncpus_filesinit(NCPUSS *) ;
static int	ncpus_filesfree(NCPUSS *,int) ;
static int	ncpus_listinit(NCPUSS *,int) ;
static int	ncpus_listfree(NCPUSS *) ;

static int	ncpus_nfcreate(NCPUSS *,const char *) ;
static int	ncpus_nfcreatecheck(NCPUSS *,
			const char *,const char *) ;
static int	ncpus_nfdestroy(NCPUSS *) ;
static int	ncpus_nfstore(NCPUSS *,const char *) ;
static int	ncpus_fexists(NCPUSS *) ;

static int	ncpus_mkvarfile(NCPUSS *) ;
static int	ncpus_wrvarfile(NCPUSS *) ;
static int	ncpus_mkind(NCPUSS *,const char *,uint (*)[3],int) ;
static int	ncpus_renamefiles(NCPUSS *) ;

#endif /* COMMENT */


/* local variables */


/* exported subroutines */


int ncpus(pr)
const char	pr[] ;
{
	SUBINFO	si ;

	int	rs = SR_OK ;
	int	n = 0 ;


	if (pr == NULL)
	    return SR_FAULT ;

	if ((rs = subinfo_start(&si,pr)) >= 0) {

	    rs = subinfo_msget(&si) ;
	    n = rs ;

	    subinfo_finish(&si) ;
	} /* end if */

#if	CF_DEBUGS
	debugprintf("ncpus_open: ret rs=%d\n",rs) ;
#endif

	return rs ;
}
/* end subroutine (ncpus) */


/* private subroutines */


static int subinfo_start(sip,pr)
SUBINFO		*sip ;
const char	*pr ;
{
	int	rs = SR_OK ;


	memset(sip,0,sizeof(SUBINFO)) ;
	sip->pr = pr ;

	return rs ;
}
/* end subroutine (subinfo_start) */


static int subinfo_msget(sip)
SUBINFO		*sip ;
{
	MSFILE_ENT	e ;

	time_t	daytime = 0 ;

	int	rs = SR_OK ;
	int	nl ;
	int	n = 0 ;

	const char	*nn ;


	if (! sip->f.msinit)
	    rs = subinfo_msopen(sip) ;

	if (rs < 0)
	    goto ret0 ;

	if (sip->f.msopen) {

	    rs = subinfo_nodename(sip) ;
	    if (rs >= 0) {

		nn = sip->nodename ;
		nl = sip->nodenamelen ;
		rs1 = msfile_match(&sip->msfile,0L,nn,nl,&e) ;
	
		if (rs1 != SR_NOTFOUND) rs = rs1 ;

	    	if (rs >= 0) {

			if (daytime == 0) daytime = time(NULL) ;

			sip->f.msold = ((daytime - e.utime) >= TO_MSOLD) ;

	    	} /* end if */

	    } /* end if (nodename) */

	} /* end if (MS opened) */

	if (rs < 0)
	    goto ret0 ;

	if ((! sip->f.msopen) || sip->f.msold) {
	    rs = subinfo_msdaemon(sip) ;
	}

ret0:
	return (rs >= 0) ? n : rs ;
}
/* end subroutine (subinfo_msget) */


static int subinfo_finish(sip)
SUBINFO		*sip ;
{
	int		rs = SR_OK ;
	int		rs1 ;

	if (sip->nodename != NULL) {
	    rs1 = uc_free(sip->nodename) ;
	    if (rs >= 0) rs = rs1 ;
	    sip->nodename = NULL ;
	}

	return rs ;
}
/* end subroutine (subinfo_finish) */


static int subinfo_msopen(sip)
SUBINFO		*sip ;
{
	int	rs = SR_OK ;
	int	rs1 ;
	int	oflags = O_RDONLY ;
	int	operms = 0666 ;

	char	fname[MAXPATHLEN + 1] ;


	if (sip->f.msinit)
	    goto ret0 ;

	sip->f.msinit = TRUE ;
	rs = mkpath3(fname,sip->pr,MSDNAME,MSFNAME) ;
	if (rs >= 0) {
	    rs1 = msfile_open(&sip->msfile,fname,oflags,operm)
	    sip->f.msfile = (rs1 >= 0) ;
	}

ret0:
	return rs ;
}
/* end subroutine (subinfo_msopen) */


static int subinfo_nodename(sip)
SUBINFO		*sip ;
{
	int	rs = SR_OK ;
	int	nlen ;

	const char	*cp ;

	char	nbuf[NODENAMELEN + 1] ;


	if (sip->nodename != NULL)
	    goto ret0 ;

	rs = getnodename(nbuf,NODENAMELEN) ;
	nlen = rs ;
	if (rs >= 0) {
	    rs = uc_mallocstrw(nbuf,nlen,&cp) ;
	    if (rs >= 0) {
		sip->nodename = cp ;
		sip->nodenamelen = nlen ;
	    }
	}

ret0:
	return rs ;
}
/* end subroutine (subinfo_nodename) */




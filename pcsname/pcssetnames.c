/* pcssetnames */

/* PCS set-names */
/* last modified %G% version %I% */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */


/* revision history:

	= 2004-03-01, David A­D­ Morano

	This subroutine was originally written.  


*/

/* Copyright © 2004 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Description:

	This subroutine sets the PCS names of a user.
	Two different types of PCS names can be set.  These are:
	1. regular name
	2. full name

	Synopsis:

	int pcssetnames(pr,nbuf,nlen,un,type)
	const char	*pr ;
	const char	*nbuf ;
	int		nlen ;
	const char	*un ;
	int		type ;

	Arguments:

	pr		program root
	nbuf		caller-supplied name buffer
	nlen		caller-supplied name buffer length
	un		username
	type		type of name to set: 0=regular, 1=full

	Returns:

	0		OK
	<0		error


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<signal.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<dlfcn.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<storebuf.h>
#include	<localmisc.h>


/* local defines */


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	sncpy3(char *,int,const char *,const char *,const char *) ;
extern int	mkpath1(char *,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	matstr(const char **,const char *,int) ;
extern int	matostr(const char **,int,const char *,int) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	optbool(const char *,int) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	pcsgetnames(const char *,char *,int,const char *,int) ;

#if	CF_DEBUGS
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif

extern char	*strwcpy(char *,const char *,int) ;


/* external variables */


/* local structures */


/* forward references */

static int	rmnames(const char *,const char *) ;
static int	setnames(const char *,const char *,const char *,int) ;


/* local variables */

static const char	*nfnames[] = {
	".name",
	".fullname",
	NULL
} ;


/* exported subroutines */


int pcssetnames(pr,nbuf,nlen,un,nt)
const char	*pr ;
const char	*nbuf ;
int		nlen ;
const char	*un ;
int		nt ;
{
	int	rs = SR_OK ;
	int	f_set ;

	char	uh[MAXPATHLEN+1] ;
	char	nfname[MAXPATHLEN + 1] ;


	if (pr == NULL) return SR_FAULT ;
	if (un == NULL) return SR_FAULT ;

	if (un[0] == '\0') return SR_INVALID ;

	rs = getuserhome(uh,MAXPATHLEN,un) ;
	if (rs >= 0) {
	    if (nt != 0) nt = 1 ;
	    rs = mkpath2(nfname,uh,nfnames[nt]) ;
	}
	if (rs >= 0) {

	    f_set = ((nbuf != NULL) && (nbuf[0] != '*')) ;
	    if (f_set) {
	        rs = setnames(pr,nfname,nbuf,nlen) ;
	    } else
	        rs = rmnames(pr,nfname) ;

	} /* end if (name-file-name) */

	return rs ;
}
/* end subroutine (pcssetnames) */


/* local subroutines */


static int rmnames(pr,nfname)
const char	*pr ;
const char	*nfname ;
{
	struct ustat	sb ;

	int	rs = SR_OK ;
	int	rs1 ;


	if (pr == NULL) return SR_FAULT ;

	rs1 = u_stat(nfname,&sb) ;

#ifdef	COMMENT
	if ((rs1 >= 0) && (! S_ISDIR(sb.st_mode)))
	    rs = u_unlink(nfname) ;
#else /* COMMENT */
	if (rs1 >= 0) 
	    rs = removes(nfname) ;
#endif /* COMMENT */

	return rs ;
}
/* end subroutine (rmnames) */


static int setnames(pr,nfname,nbuf,nlen)
const char	*pr ;
const char	*nfname ;
const char	*nbuf ;
int		nlen ;
{
	const mode_t	om = 0664 ;

	const int	of = (O_CREAT|O_TRUNC|O_WRONLY) ;
	const int	to = -1 ;

	int	rs ;
	int	size ;

	char	*p ;


	if (pr == NULL) return SR_FAULT ;

	if (nlen < 0) nlen = strlen(nbuf) ;

	size = (nlen+2) ;
	if ((rs = uc_malloc(size,&p)) >= 0) {
	    int		nl ;
	    const char	*np = (const char *) p ;
	    char	*bp = (char *) p ;

	    rs = sncpy2(bp,(size-1),nbuf,"\n") ;
	    nl = rs ;

	    if ((rs >= 0) && ((rs = uc_opene(nfname,of,om,to)) >= 0)) {
	        int	fd = rs ;

	        rs = u_write(fd,np,nl) ;

	        if (rs >= 0)
	            rs = uc_fminmod(fd,0644) ;

	        u_close(fd) ;
	    } /* end if (opened) */

	    uc_free(np) ;
	} /* end if (memory-allocations) */

	return rs ;
}
/* end subroutine (setnames) */



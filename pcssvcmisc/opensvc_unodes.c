/* opensvc_unodes */

/* PCS facility open-service (unodes) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */


/* revision history:

	= 2003-11-04, David A­D­ Morano
        This code opens a file-descriptor that returns the User-Nodes of the
        current User-Node cluster.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an open-facility-service module.

	Synopsis:

	int opensvc_unodes(pr,prn,of,om,argv,envv,to)
	const char	*pr ;
	const char	*prn ;
	int		of ;
	mode_t		om ;
	const char	**argv ;
	const char	**envv ;
	int		to ;

	Arguments:

	pr		program-root
	prn		facility name
	of		open-flags
	om		open-mode
	argv		argument array
	envv		environment array
	to		time-out

	Returns:

	>=0		file-descriptor
	<0		error


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
#include	<pcsunodes.h>
#include	<filebuf.h>
#include	<exitcodes.h>
#include	<localmisc.h>

#include	"opensvc_unodes.h"
#include	"defs.h"


/* local defines */

#define	NDF	"opensvc_unodes.deb"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	opentmpfile(const char *,int,mode_t,char *) ;
extern int	opentmp(const char *,int,mode_t) ;
extern int	isNotPresent(int) ;

extern cchar	*getourenv(cchar **,cchar *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	process(int,cchar *) ;
static int	pcsunodes_trans(PCSUNODES *,int,char *,int) ;


/* local variables */


/* exported subroutines */


/* ARGSUSED */
int opensvc_unodes(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	int		rs ;
	int		rs1 ;
	int		fd = -1 ;
	int		pipes[2] ;
	if ((rs = u_pipe(pipes)) >= 0) {
	    const int	wfd = pipes[1] ;
	    fd = pipes[0] ;
	    {
		rs = process(wfd,pr) ;
	    }
	    rs1 = u_close(wfd) ;
	    if (rs >= 0) rs = rs1 ;
	    if ((rs < 0) && (fd >= 0)) u_close(fd) ;
	} /* end if (pipes) */
	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_unodes) */


/* local subroutines */


static int process(int wfd,cchar *pr)
{
	const int	nlen = MAXHOSTNAMELEN ;
	int		rs ;
	int		rs1 ;
	char		*nbuf ;
	if ((rs = uc_malloc((nlen+1),&nbuf)) >= 0) {
	    PCSUNODES	un ;
	    if ((rs = pcsunodes_start(&un,pr)) >= 0) {
		{
		    rs = pcsunodes_trans(&un,wfd,nbuf,nlen) ;
		}
	        rs1 = pcsunodes_finish(&un) ;
	        if (rs >= 0) rs = rs1 ;
	    } /* end if (pcsunodes_finish) */
	    rs1 = uc_free(nbuf) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (m-a-f) */
	return rs ;
}
/* end subroutie (process) */


static int pcsunodes_trans(PCSUNODES *unp,int wfd,char *nbuf,int nlen)
{
	FILEBUF		fb ;
	int		rs ;
	int		rs1 ;
	if ((rs = filebuf_start(&fb,wfd,0L,0,0)) >= 0) {
	    PCSUNODES_CUR	cur ;
	    if ((rs = pcsunodes_curbegin(unp,&cur)) >= 0) {
		while ((rs = pcsunodes_enum(unp,&cur,nbuf,nlen)) >= 0) {
		    rs = filebuf_print(&fb,nbuf,rs) ;
		    if (rs < 0) break ;
		} /* end while */
		if (rs == SR_NOTFOUND) rs = SR_OK ;
		rs1 = pcsunodes_curend(unp,&cur) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (pcsunodes-cur) */
	    rs1 = filebuf_finish(&fb) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (filebuf) */
	return rs ;
}
/* end subroutine (pcsunodes_trans) */



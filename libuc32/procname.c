/* procsearch */

/* search for a process by name */
/* version %I% last modified %G% */


#define	CF_DEBUGS	0		/* non-switchable debugging */
#define	CF_DEBUG	0		/* run-time debugging */


/* revision history:

	= 1998-09-01, David A­D­ Morano
	This program was originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine searches for an active process on the current system
	by name.

	Synopsis:

	int procsarch(PROCNAME *op)

	Arguments:

	op		pointer to object

	Returns:

	<0		error
	>=0		number of group IDs returned


*******************************************************************************/


#define	PROCNAME_MASTER	0


#include	<envstandards.h>	/* MUST be first to configure */
#include	<sys/types.h>
#include	<unistd.h>		/* for |getgroups(2)| */
#include	<vsystem.h>
#include	<vecstr.h>
#include	<localmisc.h>
#include	"procname.h"		/* includes 'NGROUPS_MAX' */


/* local defines */

#define	PROCNAME_RESERVE	struct procname_reserve


/* external subroutines */


/* external variables */


/* local structures */

struct procname_reserve {
	int		ngroups ;
} ;


/* forward references */

int procname_init() ;


/* local variables */

static struct procname_reserve	procname_data ; /* zero-initialized */


/* exported subroutines */


int procsearch(vecstr *nlp)
{
	const int	of = O_RDONLY ;
	int		rs ;
	int		rs1 ;
	int		i ;
	int		c = 0 ;
	const char	*args[8] ;

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procsearch: ent\n") ;
#endif

	args[i++] = PROG_PS ;
	args[i++] = "-A" ;
	args[i++] = "-o" ;
	args[i++] = "pid,comm" ;
	args[i] = NULL ;

	if ((rs = uc_openprog(pfname,of,args,NULL)) >= 0) {
	    FILEBUF	b ;
	    int		fd = rs ;

	    if ((rs = filebuf_start(&b,fd,0L,2048,0)) >= 0) {
	        const int	llen = LINEBUFLEN ;
	        int		line = 0 ;
	        int		f_bol = TRUE ;
	        int		f_eol ;
	        char		lbuf[LINEBUFLEN+1] ;

	        while ((rs = filebuf_readline(&b,lbuf,llen,to)) > 0) {
	            int	len = rs ;

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("procsearch: l=>%t<\n",
	                    lbuf,strlinelen(lbuf,len,40)) ;
#endif

	            f_eol = (lbuf[len-1] == '\n') ;
	            if (f_eol) lbuf[--len] = '\0' ;

	            if (f_bol && f_eol && (line > 0)) {
	                rs = procsearchline(pip,nlp,lbuf,len) ;
	                c += rs ;
	            }

	            if (f_eol) line += 1 ;

	            f_bol = f_eol ;
	            if (rs < 0) break ;
	        } /* end while (reading lines) */

	        rs1 = filebuf_finish(&b) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (filebuf) */

	    rs1 = u_close(fd) ;
	    if (rs >= 0) rs = rs1 ;
	} /* end if (opened directory) */

#if	CF_DEBUG
	if (DEBUGLEVEL(4))
	    debugprintf("main/procsearch: ret rs=%d c=%u\n",rs,c) ;
#endif

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procsearch) */


/* local subroutines */


static int procsearchline(PROGINFO *pip,VECSTR *nlp,char *lbuf,int len)
{
	int		rs = SR_OK ;
	int		cl ;
	int		v ;
	int		cmdl ;
	int		c = 0 ;
	const char	*cmdp ;
	const char	*cp ;

	if (len < 0) len = strlen(lbuf) ;

	if ((cl = nextfield(lbuf,len,&cp)) > 0) {
	    if ((rs = cfdeci(cp,cl,&v)) >= 0) {
	        pid_t		pid = (pid_t) v ;
	        const int	ccl = (len-((cp+cl+1)-lbuf)) ;
	        const char	*ccp = (cp+cl+1) ;
	        if ((cmdl = sfshrink(ccp,ccl,&cmdp)) > 0) {

#if	CF_DEBUG
	            if (DEBUGLEVEL(4))
	                debugprintf("procsearchline: pid=%u c=%t\n",
	                    pid,cmdp,strlinelen(cmdp,cmdl,40)) ;
#endif

	            if ((rs = procsearchsub(pip,nlp,cmdp,cmdl)) > 0) {
	                c += 1 ;
	                rs = prochandle(pip,pid) ;
	            }

	        } /* end if (cmd) */
	    } /* end if (valid PID) */
	} /* end if (PID field) */

	return (rs >= 0) ? c : rs ;
}
/* end subroutine (procsearchline) */



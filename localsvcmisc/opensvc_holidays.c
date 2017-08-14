/* opensvc_holidays */

/* LOCAL facility open-service (HOLIDAYS) */


#define	CF_DEBUGS	0		/* non-switchable debug print-outs */
#define	CF_DEBUGN	0		/* extra-special debugging */


/* revision history:

	= 2003-11-04, David A­D­ Morano
        This code was started by taking the corresponding code from the
        TCP-family module. In retrospect, that was a mistake. Rather I should
        have started this code by using the corresponding UUX dialer module.

*/

/* Copyright © 2003 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This is an open-facility-service module.

	Synopsis:

	int opensvc_holidays(pr,prn,of,om,argv,envv,to)
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
#include	<unistd.h>
#include	<fcntl.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<filebuf.h>
#include	<holidays.h>
#include	<localmisc.h>

#include	"opensvc_holidays.h"
#include	"defs.h"


/* local defines */

#define	VARHOLIDAYSUSER	"HOLIDAYS_USERNAME"
#define	VARHOLIDAYSUID	"HOLIDAYS_UID"

#define	NDEBFNAME	"opensvc_holidays.deb"


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	sncpy2(char *,int,const char *,const char *) ;
extern int	matkeystr(const char **,const char *,int) ;
extern int	opentmpfile(cchar *,int,mode_t,char *) ;
extern int	opentmp(cchar *,int,mode_t) ;
extern int	getgroupname(char *,int,gid_t) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	localgetorg(const char *,char *,int,const char *) ;
extern int	bufprintf(char *,int,const char *,...) ;
extern int	cfdeci(const char *,int,int *) ;
extern int	isNotPresent(int) ;

extern const char	*getourenv(const char **,const char *) ;

extern char	*strwcpy(char *,const char *,int) ;


/* local structures */


/* forward references */

static int	printhols(const char *,FILEBUF *,int,const char *) ;


/* local variables */


/* exported subroutines */


int opensvc_holidays(pr,prn,of,om,argv,envv,to)
const char	*pr ;
const char	*prn ;
int		of ;
mode_t		om ;
const char	**argv ;
const char	**envv ;
int		to ;
{
	int		rs = SR_OK ;
	int		argc = 0 ;
	int		year = 2013 ;
	int		fd = -1 ;
	const char	*kn = NULL ;
	const char	*admin = NULL ;
	const char	*admins[3] ;
	const char	*query = NULL ;

	if (argv != NULL) {
	    for (argc = 0 ; argv[argc] != NULL ; argc += 1) ;
	    if ((argc >= 2) && (argv[1] != '\0')) {
		admin = argv[1] ;
	    }
	}

/* default user as necessary */

	if ((rs >= 0) && ((kn == NULL) || (kn[0] == '\0'))) kn = "-" ;

/* what administrators do we want? */

	if (rs >= 0) {
	    int	i = 0 ;
	    admins[i++] = admin ;
	    admins[i] = NULL ;
	}

/* write it out */

	if (rs >= 0) {
	if ((rs = opentmp(NULL,0,0664)) >= 0) {
	    FILEBUF	b ;
	    fd = rs ;

	    if ((rs = filebuf_start(&b,fd,0L,512,0)) >= 0) {

		rs = printhols(pr,&b,year,query) ;

		filebuf_finish(&b) ;
	    } /* end if (issue) */

	    if (rs >= 0) u_rewind(fd) ;
	    if (rs < 0) u_close(fd) ;
	} /* end if (opentmp) */
	} /* end if (ok) */

	return (rs >= 0) ? fd : rs ;
}
/* end subroutine (opensvc_holidays) */


/* local subroutines */


static int printhols(pr,fbp,year,query)
const char	*pr ;
FILEBUF		*fbp ;
int		year ;
const char	*query ;
{
	HOLIDAYS	h ;
	HOLIDAYS_CUR	c ;
	HOLIDAYS_CITE	q ;
	int		rs ;
	int		wlen = 0 ;

#ifdef	COMMENT
	q.something = year? ;
#else
	memset(&q,0,sizeof(HOLIDAYS_CITE)) ;
#endif

	    if ((rs = holidays_open(&h,pr,year,NULL)) >= 0) {
		if ((rs = holidays_curbegin(&h,&c)) >= 0) {
		    const int	vlen = VBUFLEN ;
		    int		vl ;
		    char	vbuf[VBUFLEN+1] ;
		    while (rs >= 0) {
	                vl = holidays_fetchcite(&h,&q,&c,vbuf,vlen) ;
		        if (vl == SR_NOTFOUND) break ;
			rs = vl ;
			if (rs >= 0) {
			    rs = filebuf_print(fbp,vbuf,vl) ;
			    wlen += rs ;
			}
		    } /* end while */
		    holidays_curend(&h,&c) ;
		} /* end if (cursor) */
		holidays_close(&h) ;
	    } /* end if (holiday) */

	return (rs >= 0) ? wlen : rs ;
}
/* end subroutine (printhols) */



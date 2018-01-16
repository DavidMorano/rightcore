/* localsetnetload */

/* set the LOCAL network-load (NETLOAD) */


#define	CF_DEBUGS	0		/* compile-time debugging */
#define	CF_UPROGDATA	1		/* use |uprogdata_xxx(3uc)| */


/* revision history:

	= 1998-05-01, David A­D­ Morano
	This subroutine is originally written.

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	This subroutine sets the LOCAL system network-load.

	Synopsis:

	int localsetnetload(pr,sbuf,slen)
	const char	pr[] ;
	const char	sbuf[] ;
	char		slen ;

	Arguments:

	pr		program root
	sbuf		caller supplied buffer w/ value to set
	slen		length of caller supplied buffer

	Returns:

	>=0		OK
	<0		error


	Notes:

	Q. Why the program-cache?
	A. It serves as a short-cut for future "gets."


*******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<uprogdata.h>
#include	<localmisc.h>


/* local defines */

#ifndef	VARNETLOAD
#define	VARNETLOAD	"NETLOAD"
#endif

#define	ETCDNAME	"etc"
#define	VARDNAME	"var"
#define	NETLOADFNAME	"netload"
#define	TO_TTL		(5*60)


/* external subroutines */

extern int	sncpy1(char *,int,const char *) ;
extern int	mkpath2(char *,const char *,const char *) ;
extern int	mkpath3(char *,const char *,const char *,const char *) ;
extern int	getnodedomain(char *,char *) ;
extern int	getuserhome(char *,int,const char *) ;
extern int	localgetorg(const char *,char *,int,const char *) ;
extern int	readfileline(char *,int,const char *) ;
extern int	isNotPresent(int) ;


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int localsetnetload(cchar *pr,cchar *sbuf,int slen)
{
	const int	di = UPROGDATA_DNETLOAD ;
	const int	ttl = TO_TTL ;
	int		rs ;
	int		rs1 ;
	int		rc = 0 ;
	const char	*vardname = VARDNAME ;
	const char	*netloadname = NETLOADFNAME ;
	char		tfname[MAXPATHLEN+1] ;

	if (pr == NULL) return SR_FAULT ;
	if (sbuf == NULL) return SR_FAULT ;

	if (pr[0] == '\0') return SR_INVALID ;

	if ((rs = mkpath3(tfname,pr,vardname,netloadname)) >= 0) {
	    bfile		dfile, *dfp = &dfile ;
	    const mode_t	om = 0664 ;
	    if ((rs = bopen(dfp,tfname,"wct",om)) >= 0) {
		cchar	*fmt = "# NETLOAD (Machine Network-Load)" ;
		if ((rs = bprintln(dfp,fmt,-1)) >= 0) {
	            if ((rs = bprintln(dfp,sbuf,slen)) >= 0) {
		        rs = uprogdata_set(di,sbuf,slen,ttl) ;
			rc = rs ;
		    }
		}
		rs1 = bclose(dfp) ;
		if (rs >= 0) rs = rs1 ;
	    } /* end if (file-write) */
	} /* end if (mkpath) */

#if	CF_DEBUGS
	debugprintf("localsetnetload: ret rs=%d rc=%u\n",rs,rc) ;
#endif

	return (rs >= 0) ? rc : rs ;
}
/* end subroutine (localsetnetload) */



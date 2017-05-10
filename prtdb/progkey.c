/* progkey */

/* process a key */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */
#define	CF_DEBUG	0		/* run-time debug print-outs */
#define	CF_LPGET	1		/* test LPGET */


/* revision history:

	= 1998-03-01, David A­D­ Morano

	The subroutine was adapted from others programs that
	did similar types of functions.


*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/******************************************************************************

	This subroutine processes one key.

	Synopsis:

	int progkey(pip,printer,keyname)
	struct proginfo	*pip ;
	const char	printer[] ;
	const char	keyname[] ;

	Arguments:

	pip		program information pointer
	printer		printer
	keyname		key name to test for

	Returns:

	>=0		good
	<0		error


******************************************************************************/


#include	<envstandards.h>	/* MUST be first to configure */

#include	<sys/types.h>
#include	<sys/param.h>
#include	<sys/stat.h>
#include	<signal.h>
#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<bfile.h>
#include	<localmisc.h>

#include	"config.h"
#include	"defs.h"


/* local defines */

#ifndef	KEYBUFLEN
#define	KEYBUFLEN	MAXNAMELEN
#endif

#ifndef	VBUFLEN
#define	VBUFLEN		MAXNAMELEN
#endif


/* external subroutines */

extern int	snwcpy(char *,int,const char *,int) ;
extern int lpgetout(struct proginfo *,const char *,char *,int,const char *) ;

#if	CF_DEBUGS || CF_DEBUG
extern int	debugprintf(const char *,...) ;
extern int	strlinelen(const char *,int,int) ;
#endif


/* external variables */


/* local structures */


/* forward references */


/* local variables */


/* exported subroutines */


int progkey(pip,printer,kp,kl)
struct proginfo	*pip ;
const char	printer[] ;
const char	kp[] ;
int		kl ;
{
	const int	vlen = VBUFLEN ;

	int	rs ;
	int	vl = 0 ;

	const char	*pp ;

	char	keyname[KEYBUFLEN+1] ;
	char	vbuf[VBUFLEN + 1] ;


	if (printer == NULL) return SR_FAULT ;
	if (kp == NULL) return SR_FAULT ;

	rs = snwcpy(keyname,KEYBUFLEN,kp,kl) ;
	if (rs < 0) goto ret0 ;

#if	CF_DEBUG
	debugprintf("progkey: printer=%s keyname=%s\n",printer,keyname) ;
#endif

/* access the front database (local and system) */

	if ((vl == 0) && pip->f.pdbopen) {

	    pp = (strcmp(printer,DEFPRINTER) == 0) ? "default" : printer ;

	    rs = pdb_fetch(&pip->db,pp,keyname,vbuf,vlen) ;
	    vl = rs ;

#if	CF_DEBUG
	debugprintf("progkey: pdb_fetch() rs=%d\n",rs) ;
#endif

	    if (rs == SR_NOTFOUND) {
		rs = SR_OK ;
		vl = 0 ;
	    }

	} /* end if (PDB) */

/* access the backend database (system) */

#if	CF_LPGET
	if ((rs >= 0) && (vl == 0)) {

	    pp = (strcmp(printer,"default") == 0) ? DEFPRINTER : printer ;

	    rs = lpgetout(pip,pp,vbuf,vlen,keyname) ;
	    vl = rs ;

#if	CF_DEBUG
	if (DEBUGLEVEL(3)) {
	debugprintf("progkey: lpgetout() rs=%d\n",rs) ;
	debugprintf("progkey: v=>%t<\n",vbuf,strlinelen(vbuf,vl,40)) ;
	}
#endif

	}
#endif /* CF_LPGET */

/* print out result */

	if (rs >= 0) {
	    rs = bprintf(pip->ofp,"%t\n",vbuf,vl) ;
	}

ret0:

#if	CF_DEBUG
	if (DEBUGLEVEL(2))
	    debugprintf("progkey: ret rs=%d vl=%u\n",rs,vl) ;
#endif

	return (rs >= 0) ? vl : rs ;
}
/* end subroutine (progkey) */




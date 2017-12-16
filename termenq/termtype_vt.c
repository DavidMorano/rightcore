/* vtxx0 */

/* discriminate among VT52x models */


#define	CF_DEBUGS	0		/* compile-time debug print-outs */


/* revision history:

	= 2000-07-19, David A­D­ Morano
	This subroutine was written for Rightcore Network Services (RNS).

*/

/* Copyright © 1998 David A­D­ Morano.  All rights reserved. */

/*******************************************************************************

	Discrminate among VTxx0 models.


*******************************************************************************/


#include	<envstandards.h>

#include	<sys/types.h>
#include	<sys/param.h>
#include	<limits.h>
#include	<stdlib.h>
#include	<string.h>

#include	<vsystem.h>
#include	<localmisc.h>

#include	"termtype.h"


/* local defines */

#define	U	SHORT_MIN


/* external subroutines */

extern int	sncpy1(char *,int,cchar *) ;

#if	CF_DEBUGS
extern int	debugprint(cchar *,int) ;
extern int	debugprintf(cchar *,...) ;
extern int	strlinelen(cchar *,int,int) ;
#endif

extern char	*strwcpy(char *,char *,int) ;
extern char	*strnwcpy(char *,int,cchar *,int) ;


/* local structures */


/* forward references */


/* local variables */

static const struct termtype	types[] = {
	{ "screen",	{ 1, 2, U, U },		{ 83, 9999, 0, U } },
	{ "vt100",	{ 1, 2, U, U },		{ U, U, U, U } },
	{ "vt101",	{ 1, 0, U, U },		{ U, U, U, U } },
	{ "vt102",	{ 6, U, U, U },		{ U, U, U, U } },
	{ "vt220",	{ 62, -9, U, U },	{ U, U, U, U } },
	{ "vt220int",	{ 62, 9, U, U },	{ U, U, U, U } },
	{ "vt320",	{ 63, -9, U, U },	{ U, U, U, U } },
	{ "vt320int",	{ 63, 9, U, U },	{ U, U, U, U } },
	{ "vt420",	{ 64, U, U, U },	{ U, U, U, U } },
	{ "vt520",	{ 65, -22, U, U },	{ U, U, U, U } },
	{ "vt525",	{ 65, 22, U, U },	{ 64, U, U, U } },
	{ NULL,		{ 0, 0, 0, 0 },		{ 0, 0, 0, 0 } }
} ;


/* exported subroutines */


int termtype_vt(char *rbuf,int rlen, const short *pvp,const short *svp)
{
	int		rs = SR_OK ;
	int		si ;
	if (rbuf == NULL) return SR_FAULT ;
	if (pvp == NULL) return SR_FAULT ;
	if (svp == NULL) return SR_FAULT ;
	rbuf[0] = 0 ;
	if ((si = termtype(types,pvp,svp)) >= 0) {
	    rs = sncpy1(rbuf,rlen,types[si].name) ;
	}
	return rs ;
}
/* end subroutine (termtype_vt) */


